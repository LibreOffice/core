#!/usr/bin/env python3
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Compiles a Lightproof rule package into the binary .lpr format read by
# liblightproof.  Run at build time only; the shipped product needs no
# Python to use the result.
#
# Usage: lpcompile.py <dictionaries-dir> <package> <output.lpr>
#        lpcompile.py --emit-schema <dictionaries-dir> <package>
#
#   <package> is the Lightproof package name, which is the name used in the
#   lightproof_<pkg>.py file names: en, hu_HU, pt_BR, ru_RU.
#
# --emit-schema prints the officecfg group for the package's options, for
# pasting under GrammarChecking/SentenceChecking in
# officecfg/registry/schema/org/openoffice/Office/Linguistic.xcs. The schema
# is committed rather than generated because integrators set these keys, so
# the node names are an interface and should not move under anyone's feet.
#
# --emit-labels prints the same options as settingLabels entries for
# browser/admin/src/integrator/AdminIntegratorSettings.ts.

import argparse
import ast
import os
import struct
import sys

LPR_MAGIC = b"LPROOF\0\0"
LPR_VERSION = 2

# Opcodes.  The numbering is part of the file format: append, never reorder.
OP_END = 0
OP_PUSH_STR = 1
OP_PUSH_INT = 2
OP_PUSH_BOOL = 3
OP_PUSH_NULL = 4
OP_PUSH_CONST = 5
OP_LOAD_TEXT = 6
OP_LOAD_SENTENCE = 7
OP_LOAD_LOCALE = 8
OP_LOCALE_LANG = 9
OP_LOCALE_COUNTRY = 10
OP_GROUP = 11
OP_MSTART = 12
OP_MEND = 13
OP_NOT = 14
OP_JMP_IF_FALSE_KEEP = 15
OP_JMP_IF_TRUE_KEEP = 16
OP_JMP = 17
OP_POP = 18
OP_EQ = 19
OP_NE = 20
OP_IN = 21
OP_NOT_IN = 22
OP_CONCAT = 23
OP_LOWER = 24
OP_UPPER = 25
OP_CAPITALIZE = 26
OP_REPLACE = 27
OP_TRANSLATE = 28
OP_SLICE = 29
OP_INDEX = 30
OP_RE_SEARCH = 31
OP_RE_MATCH = 32
OP_RE_SUB = 33
OP_CALL = 34
OP_TRUTHY = 35

# Host function ids, also part of the file format.
HOST_FUNCS = {
    "option": (0, 2),
    "spell": (1, 2),
    "morph": (2, 3),
    "affix": (3, 3),
    "stem": (4, 2),
    "generate": (5, 3),
    "suggest": (6, 2),
    "word": (7, 2),
    "wordmin": (8, 2),
    "calc": (9, 2),
    "measurement": (10, 6),
    "suggest_foreign": (11, 2),
}

# Regex flags, matching the UREGEX_* values liblightproof passes to ICU.
LPR_CASE_INSENSITIVE = 0x0002


class Unsupported(Exception):
    """A construct this compiler does not lower yet."""


# ---------------------------------------------------------------- reading

def read_rule_table(path):
    src = open(path, encoding="utf-8").read()
    marker = src.index("dic = ")
    return ast.literal_eval(src[marker + len("dic = "):])


def read_module_assignments(path, names):
    """Pick named module-level literal assignments out of a Python file.

    Handles both "lopts = {}" and the "lopts['en'] = [...]" form the option
    files use.  Only literals are evaluated, so the rule implementation is
    never imported or executed.
    """
    tree = ast.parse(open(path, encoding="utf-8").read())
    found = {}
    for node in tree.body:
        if not isinstance(node, ast.Assign):
            continue
        try:
            value = ast.literal_eval(node.value)
        except ValueError:
            continue
        for target in node.targets:
            if isinstance(target, ast.Name) and target.id in names:
                found[target.id] = value
            elif (isinstance(target, ast.Subscript)
                    and isinstance(target.value, ast.Name)
                    and target.value.id in names
                    and isinstance(target.slice, ast.Constant)):
                found.setdefault(target.value.id, {})[target.slice.value] = value
    return found


def read_data_tables(path):
    """The module-level tables the rule conditions look words up in.

    Recognises the three shapes the packages use: set([...]) for word sets,
    a dict literal for lookup tables, and re.compile(...) for the auxiliary
    patterns. Nothing is imported or executed.
    """
    tree = ast.parse(open(path, encoding="utf-8").read())
    tables = {}
    for node in tree.body:
        if not isinstance(node, ast.Assign) or not isinstance(node.targets[0], ast.Name):
            continue
        name = node.targets[0].id
        value = node.value
        if isinstance(value, ast.Call) and isinstance(value.func, ast.Name) \
                and value.func.id == "set" and len(value.args) == 1:
            try:
                tables[name] = ("set", sorted(set(ast.literal_eval(value.args[0]))))
            except ValueError:
                pass
        elif isinstance(value, ast.Call) and isinstance(value.func, ast.Attribute) \
                and value.func.attr == "compile" and value.args:
            try:
                tables[name] = ("regex", ast.literal_eval(value.args[0]))
            except ValueError:
                pass
        elif isinstance(value, (ast.Dict, ast.List, ast.Set)):
            try:
                literal = ast.literal_eval(value)
            except ValueError:
                continue
            if isinstance(literal, dict):
                tables[name] = ("map", sorted(literal.items()))
            else:
                tables[name] = ("set", sorted(set(literal)))
    return tables


def read_licence(dictdir, pkg):
    """The package licence notice, which the .lpr carries into the product."""
    for name in ("README_Lightproof_%s.txt" % pkg, "README_lightproof_%s.txt" % pkg):
        path = os.path.join(dictdir, name)
        if os.path.exists(path):
            text = open(path, encoding="utf-8", errors="replace").read().strip()
            return " ".join(text.split())
    return ""


# ---------------------------------------------------------------- regexes

def translate_regex(pattern):
    """Python re source to ICU regex source.

    Returns (icu_pattern, flags, {group name: group number}).  Named groups
    become plain capturing groups and the names are resolved here, so the
    runtime never asks ICU for a group by name.
    """
    flags = 0
    if pattern.startswith("(?iu)"):
        flags |= LPR_CASE_INSENSITIVE
        pattern = pattern[5:]
    elif pattern.startswith("(?u)"):
        pattern = pattern[4:]

    out = []
    names = {}
    ngroups = 0
    i = 0
    n = len(pattern)
    while i < n:
        c = pattern[i]
        if c == "\\":
            out.append(pattern[i:i + 2])
            i += 2
            continue
        if c == "[":
            j = i + 1
            body = []
            if j < n and pattern[j] == "^":
                body.append("^")
                j += 1
            if j < n and pattern[j] == "]":
                body.append("\\]")
                j += 1
            while j < n and pattern[j] != "]":
                if pattern[j] == "\\":
                    body.append(pattern[j:j + 2])
                    j += 2
                    continue
                # ICU reads a nested set, a set intersection and a string in
                # a set where Python re has three plain characters.
                if pattern[j] in "[&{":
                    body.append("\\" + pattern[j])
                else:
                    body.append(pattern[j])
                j += 1
            if j >= n:
                raise Unsupported("unterminated character class")
            out.append("[" + "".join(body) + "]")
            i = j + 1
            continue
        if c == "(":
            if pattern.startswith("(?P<", i):
                end = pattern.index(">", i)
                ngroups += 1
                names[pattern[i + 4:end]] = ngroups
                out.append("(")
                i = end + 1
                continue
            if pattern.startswith("(?P=", i):
                raise Unsupported("named backreference")
            if pattern.startswith("(?", i):
                out.append(pattern[i:i + 2])
                i += 2
                continue
            ngroups += 1
            out.append("(")
            i += 1
            continue
        if pattern.startswith("(?u)", i) or pattern.startswith("(?iu)", i):
            raise Unsupported("inline unicode flag away from the start")
        out.append(c)
        i += 1

    return "".join(out), flags, names


# ---------------------------------------------------------------- lowering

class Lowerer:
    """Lowers a Python expression to .lpr bytecode.

    Only the subset the rule packages actually use is accepted; anything
    else raises Unsupported so the build fails loudly rather than shipping a
    rule that silently never fires.
    """

    # Stands in for a missing slice bound, which Python writes as nothing.
    SLICE_OPEN = -0x80000000

    def __init__(self, writer, tables=None, group_names=None):
        self.w = writer
        self.tables = tables or {}
        self.group_names = group_names or {}
        self.code = bytearray()

    def emit(self, opcode, *operands):
        self.code.append(opcode)
        for operand in operands:
            self.code += struct.pack("<I", operand & 0xFFFFFFFF)

    def lower(self, source):
        tree = ast.parse(source.strip(), mode="eval")
        self.visit(tree.body)
        self.emit(OP_END)
        return bytes(self.code)

    def visit(self, node):
        handler = getattr(self, "v_" + type(node).__name__, None)
        if handler is None:
            raise Unsupported(type(node).__name__)
        handler(node)

    def v_Constant(self, node):
        value = node.value
        if value is None:
            self.emit(OP_PUSH_NULL)
        elif isinstance(value, bool):
            self.emit(OP_PUSH_BOOL, 1 if value else 0)
        elif isinstance(value, int):
            self.emit(OP_PUSH_INT, value)
        elif isinstance(value, str):
            self.emit(OP_PUSH_STR, self.w.string(value))
        else:
            raise Unsupported("constant of type %s" % type(value).__name__)

    def v_Name(self, node):
        if node.id == "LOCALE":
            self.emit(OP_LOAD_LOCALE)
        elif node.id == "TEXT":
            self.emit(OP_LOAD_TEXT)
        elif node.id == "s":
            self.emit(OP_LOAD_SENTENCE)
        elif node.id in self.tables:
            self.emit(OP_PUSH_CONST, self.w.const_for_table(node.id, self.tables[node.id]))
        else:
            raise Unsupported("name %s" % node.id)

    def v_List(self, node):
        try:
            values = [ast.literal_eval(element) for element in node.elts]
        except ValueError:
            raise Unsupported("list of non-literals")
        if not all(isinstance(value, str) for value in values):
            raise Unsupported("list of non-strings")
        self.emit(OP_PUSH_CONST, self.w.const_set(values))

    def v_Tuple(self, node):
        self.v_List(node)

    def v_BinOp(self, node):
        if not isinstance(node.op, ast.Add):
            raise Unsupported("binary %s" % type(node.op).__name__)
        self.visit(node.left)
        self.visit(node.right)
        self.emit(OP_CONCAT)

    def v_Compare(self, node):
        if len(node.ops) != 1:
            raise Unsupported("chained comparison")
        opcodes = { ast.In: OP_IN, ast.NotIn: OP_NOT_IN, ast.Eq: OP_EQ, ast.NotEq: OP_NE }
        opcode = opcodes.get(type(node.ops[0]))
        if opcode is None:
            raise Unsupported("comparison %s" % type(node.ops[0]).__name__)
        self.visit(node.left)
        self.visit(node.comparators[0])
        self.emit(opcode)

    def v_Subscript(self, node):
        if isinstance(node.slice, ast.Slice):
            if node.slice.step is not None:
                raise Unsupported("slice with a step")
            self.visit(node.value)
            self.emit(OP_SLICE, self.bound(node.slice.lower), self.bound(node.slice.upper))
            return
        self.visit(node.value)
        self.visit(node.slice)
        self.emit(OP_INDEX)

    def bound(self, node):
        if node is None:
            return self.SLICE_OPEN
        try:
            value = ast.literal_eval(node)
        except ValueError:
            raise Unsupported("slice bound is not a literal")
        if not isinstance(value, int):
            raise Unsupported("slice bound is not an integer")
        return value

    def group_index(self, node):
        """The capture group a m.group()/start()/end() argument names."""
        try:
            value = ast.literal_eval(node)
        except ValueError:
            raise Unsupported("dynamic group reference")
        if isinstance(value, int):
            return value
        index = self.group_names.get(value)
        if index is None:
            raise Unsupported("unknown group name %s" % value)
        return index

    def v_Attribute(self, node):
        if isinstance(node.value, ast.Name) and node.value.id == "LOCALE":
            if node.attr == "Language":
                self.emit(OP_LOCALE_LANG)
                return
            if node.attr == "Country":
                self.emit(OP_LOCALE_COUNTRY)
                return
        raise Unsupported("attribute .%s" % node.attr)

    def v_UnaryOp(self, node):
        if isinstance(node.op, ast.Not):
            self.visit(node.operand)
            self.emit(OP_NOT)
            return
        if isinstance(node.op, (ast.USub, ast.UAdd)):
            try:
                value = ast.literal_eval(node)
            except ValueError:
                raise Unsupported("sign on a computed value")
            if not isinstance(value, int):
                raise Unsupported("sign on a non-integer")
            self.emit(OP_PUSH_INT, value)
            return
        raise Unsupported("unary %s" % type(node.op).__name__)

    def v_BoolOp(self, node):
        # Python semantics: the operand value is the result, not a boolean.
        jump = OP_JMP_IF_FALSE_KEEP if isinstance(node.op, ast.And) else OP_JMP_IF_TRUE_KEEP
        patches = []
        for index, value in enumerate(node.values):
            if index:
                self.emit(OP_POP)
            self.visit(value)
            if index < len(node.values) - 1:
                patches.append(len(self.code))
                self.emit(jump, 0)
        for site in patches:
            struct.pack_into("<I", self.code, site + 1, len(self.code))

    # Method calls the rules make on a match, on a string, or on one of the
    # module-level patterns.
    STRING_METHODS = { "lower": OP_LOWER, "upper": OP_UPPER, "capitalize": OP_CAPITALIZE }
    REGEX_METHODS = { "search": OP_RE_SEARCH, "match": OP_RE_MATCH }

    def v_Call(self, node):
        if isinstance(node.func, ast.Attribute):
            self.lower_method(node)
            return
        if not isinstance(node.func, ast.Name):
            raise Unsupported("call of a computed value")
        entry = HOST_FUNCS.get(node.func.id)
        if entry is None:
            raise Unsupported("call to %s" % node.func.id)
        func_id, max_args = entry
        if node.keywords or len(node.args) > max_args:
            raise Unsupported("call shape of %s" % node.func.id)
        for arg in node.args:
            self.visit(arg)
        self.code.append(OP_CALL)
        self.code.append(func_id)
        self.code.append(len(node.args))

    def lower_method(self, node):
        method = node.func.attr
        receiver = node.func.value

        if isinstance(receiver, ast.Name) and receiver.id == "m":
            if method not in ("group", "start", "end") or len(node.args) != 1:
                raise Unsupported("m.%s()" % method)
            opcode = { "group": OP_GROUP, "start": OP_MSTART, "end": OP_MEND }[method]
            self.emit(opcode, self.group_index(node.args[0]))
            return

        if method in self.STRING_METHODS:
            if node.args:
                raise Unsupported("%s() with arguments" % method)
            self.visit(receiver)
            self.emit(self.STRING_METHODS[method])
            return

        if method in self.REGEX_METHODS:
            if not isinstance(receiver, ast.Name) or receiver.id not in self.tables:
                raise Unsupported("%s() on a computed pattern" % method)
            if len(node.args) != 1:
                raise Unsupported("%s() with %d arguments" % (method, len(node.args)))
            self.visit(node.args[0])
            self.emit(self.REGEX_METHODS[method],
                      self.w.const_for_table(receiver.id, self.tables[receiver.id]))
            return

        raise Unsupported("method .%s()" % method)


# ---------------------------------------------------------------- writing

CONST_SET = 0
CONST_MAP = 1
CONST_REGEX = 2


class Writer:
    def __init__(self):
        self.strings = bytearray(b"\0")
        self.string_offsets = {"": 0}
        self.code = bytearray()
        # Each entry is (type, count, data, flags); data points into
        # const_data for sets and maps, and into the string blob for regexes.
        self.consts = []
        self.const_data = bytearray()
        self.const_ids = {}

    def const_set(self, values, key=None):
        """A sorted string set, searched by UTF-8 byte order at run time.

        Python sorts strings by code point and UTF-8 preserves that order, so
        the runtime can binary-search the encoded bytes directly.
        """
        values = sorted(set(values))
        if key is None:
            key = ("set",) + tuple(values)
        if key in self.const_ids:
            return self.const_ids[key]
        offset = len(self.const_data)
        for value in values:
            self.const_data += struct.pack("<I", self.string(value))
        index = len(self.consts)
        self.consts.append((CONST_SET, len(values), offset, 0))
        self.const_ids[key] = index
        return index

    def const_map(self, items, key=None):
        items = sorted(items)
        if key is None:
            key = ("map",) + tuple(items)
        if key in self.const_ids:
            return self.const_ids[key]
        offset = len(self.const_data)
        for name, value in items:
            self.const_data += struct.pack("<II", self.string(name), self.string(value))
        index = len(self.consts)
        self.consts.append((CONST_MAP, len(items), offset, 0))
        self.const_ids[key] = index
        return index

    def const_regex(self, pattern, key=None):
        if key is None:
            key = ("regex", pattern)
        if key in self.const_ids:
            return self.const_ids[key]
        icu_pattern, flags, _names = translate_regex(pattern)
        index = len(self.consts)
        self.consts.append((CONST_REGEX, 0, self.string(icu_pattern), flags))
        self.const_ids[key] = index
        return index

    def const_for_table(self, name, table):
        kind, value = table
        if kind == "set":
            return self.const_set(value, key=("named", name))
        if kind == "map":
            return self.const_map(value, key=("named", name))
        return self.const_regex(value, key=("named", name))

    def string(self, value):
        offset = self.string_offsets.get(value)
        if offset is None:
            offset = len(self.strings)
            self.strings += value.encode("utf-8") + b"\0"
            self.string_offsets[value] = offset
        return offset

    def bytecode(self, blob):
        """Store a bytecode blob and return its offset, biased by one so that
        zero can mean 'no expression'."""
        offset = len(self.code)
        self.code += blob
        return offset + 1


def align4(blob):
    while len(blob) % 4:
        blob += b"\0"
    return blob


def compile_package(dictdir, pkg, out_path, verbose=False):
    rules = read_rule_table(os.path.join(dictdir, "pythonpath", "lightproof_%s.py" % pkg))
    impl = read_module_assignments(
        os.path.join(dictdir, "pythonpath", "lightproof_impl_%s.py" % pkg),
        {"locales", "name"})
    opts = read_module_assignments(
        os.path.join(dictdir, "pythonpath", "lightproof_opts_%s.py" % pkg),
        {"lopts", "lopts_default"})
    tables = read_data_tables(
        os.path.join(dictdir, "pythonpath", "lightproof_impl_%s.py" % pkg))

    locales = sorted(impl.get("locales", {}).keys())
    if not locales:
        sys.exit("lpcompile: %s: no locales found" % pkg)
    option_names = opts.get("lopts", {}).get(pkg, [])
    option_defaults = set(opts.get("lopts_default", {}).get(pkg, []))

    writer = Writer()
    group_name_table = []
    rule_records = []
    dropped = 0

    for index, rule in enumerate(rules):
        pattern, replacement, message, condition, ngroup = rule[:5]
        try:
            icu_pattern, flags, names = translate_regex(pattern)
        except Unsupported as error:
            sys.stderr.write("lpcompile: %s rule %d: bad pattern (%s), dropped\n"
                             % (pkg, index, error))
            dropped += 1
            continue

        def lower_or_die(source, what):
            lowerer = Lowerer(writer, tables, names)
            try:
                return writer.bytecode(lowerer.lower(source))
            except (Unsupported, SyntaxError) as error:
                sys.exit("lpcompile: %s rule %d: %s: %s" % (pkg, index, what, error))

        condition_code = 0
        if isinstance(condition, str) and condition.strip():
            condition_code = lower_or_die(condition, "condition")
        elif condition is False or condition is None:
            condition_code = 0
        elif condition is not True and not isinstance(condition, str):
            sys.exit("lpcompile: %s rule %d: unexpected condition %r" % (pkg, index, condition))

        replacement_code = 0
        replacement_off = 0
        if isinstance(replacement, str) and replacement.startswith("="):
            replacement_code = lower_or_die(replacement[1:], "replacement")
        else:
            replacement_off = writer.string(replacement)

        message_code = 0
        message_off = 0
        if isinstance(message, str) and message.startswith("="):
            message_code = lower_or_die(message[1:], "message")
        else:
            message_off = writer.string(message)

        group_first = len(group_name_table)
        for name in sorted(names):
            group_name_table.append((writer.string(name), names[name]))

        rule_records.append((
            writer.string(icu_pattern), flags,
            replacement_off, replacement_code,
            message_off, message_code,
            condition_code, ngroup,
            group_first, len(names),
        ))

    if dropped:
        sys.stderr.write("lpcompile: %s: %d rule(s) dropped\n" % (pkg, dropped))

    const_records = [struct.pack("<4I", *entry) for entry in writer.consts]
    option_records = [(writer.string(name), 1 if name in option_defaults else 0)
                      for name in option_names]
    locale_offsets = [writer.string(tag) for tag in locales]
    pkg_off = writer.string(pkg)
    name_off = writer.string(impl.get("name", "Lightproof grammar checker"))
    licence_off = writer.string(read_licence(dictdir, pkg))

    # Header: magic, version, then offset/count pairs for each section.
    header_size = 8 + 4 * 20
    sections = []
    cursor = header_size

    def place(blob):
        nonlocal cursor
        blob = align4(blob)
        offset = cursor
        sections.append(blob)
        cursor += len(blob)
        return offset

    locales_off = place(b"".join(struct.pack("<I", o) for o in locale_offsets))
    options_off = place(b"".join(struct.pack("<II", *o) for o in option_records))
    rules_off = place(b"".join(struct.pack("<10I", *r) for r in rule_records))
    groups_off = place(b"".join(struct.pack("<II", *g) for g in group_name_table))
    code_off = place(bytes(writer.code))
    consts_off = place(b"".join(const_records))
    const_data_off = place(bytes(writer.const_data))
    strings_off = place(bytes(writer.strings))

    header = LPR_MAGIC + struct.pack(
        "<20I",
        LPR_VERSION, 0,
        pkg_off, name_off, licence_off,
        len(locale_offsets), locales_off,
        len(option_records), options_off,
        len(rule_records), rules_off,
        len(group_name_table), groups_off,
        code_off, len(writer.code),
        len(writer.consts), consts_off,
        strings_off, len(writer.strings),
        const_data_off)

    with open(out_path, "wb") as out:
        out.write(header)
        for blob in sections:
            out.write(blob)

    # Only when asked: the build announces the file it is making already,
    # and a line per package on every build is noise in a silent make.
    if verbose:
        sys.stderr.write("lpcompile: %s: %d rules, %d options, %d constants, %d bytes\n"
                         % (pkg, len(rule_records), len(option_records), len(writer.consts), cursor))


def read_option_labels(dictdir, pkg):
    """The English option labels from the package's own dialog properties."""
    labels = {}
    path = os.path.join(dictdir, "dialog", "%s_en_US.properties" % pkg)
    if not os.path.exists(path):
        return labels
    for line in open(path, encoding="utf-8"):
        if "=" in line and not line.startswith("#"):
            name, _, label = line.partition("=")
            # The dialog properties are Java-style, with \uXXXX escapes.
            labels[name.strip()] = label.strip().encode("ascii", "backslashreplace") \
                .decode("unicode_escape")
    return labels


def escape(text):
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def emit_schema(dictdir, pkg):
    opts = read_module_assignments(
        os.path.join(dictdir, "pythonpath", "lightproof_opts_%s.py" % pkg),
        {"lopts", "lopts_default"})
    tables = read_data_tables(
        os.path.join(dictdir, "pythonpath", "lightproof_impl_%s.py" % pkg))
    names = opts.get("lopts", {}).get(pkg, [])
    defaults = set(opts.get("lopts_default", {}).get(pkg, []))
    labels = read_option_labels(dictdir, pkg)

    out = ['        <group oor:name="%s">' % pkg,
           "          <info>",
           "            <desc>Sentence checking options for the %s rules.</desc>" % pkg,
           "          </info>"]
    for name in names:
        label = labels.get(name, name)
        out += ['          <prop oor:name="%s" oor:type="xs:boolean" oor:nillable="false">' % name,
                "            <info>",
                "              <desc>%s</desc>" % escape(label),
                "              <label>%s</label>" % escape(label),
                "            </info>",
                "            <value>%s</value>" % ("true" if name in defaults else "false"),
                "          </prop>"]
    out.append("        </group>")
    print("\n".join(out))


def emit_labels(dictdir, pkg):
    opts = read_module_assignments(
        os.path.join(dictdir, "pythonpath", "lightproof_opts_%s.py" % pkg),
        {"lopts"})
    labels = read_option_labels(dictdir, pkg)
    # The key is the package and the option, because packages share option
    # names and the dialog's label map is flat.
    print("\t\t// %s" % pkg)
    for name in opts.get("lopts", {}).get(pkg, []):
        print("\t\t'%s-%s': _('%s')," % (pkg, name, labels.get(name, name).replace("'", "\\'")))


def main():
    parser = argparse.ArgumentParser(description="Compile a Lightproof rule package")
    parser.add_argument("--emit-schema", action="store_true",
                        help="print the officecfg option group instead of compiling")
    parser.add_argument("--emit-labels", action="store_true",
                        help="print the option labels for the browser settings dialog")
    parser.add_argument("--verbose", action="store_true",
                        help="report what was compiled; warnings are printed either way")
    parser.add_argument("dictdir", help="the package's directory under dictionaries/")
    parser.add_argument("package", help="package name, e.g. ru_RU")
    parser.add_argument("output", nargs="?", help="the .lpr file to write")
    args = parser.parse_args()

    if args.emit_schema:
        emit_schema(args.dictdir, args.package)
    elif args.emit_labels:
        emit_labels(args.dictdir, args.package)
    elif args.output:
        compile_package(args.dictdir, args.package, args.output, args.verbose)
    else:
        parser.error("an output file is required when compiling")


if __name__ == "__main__":
    main()
