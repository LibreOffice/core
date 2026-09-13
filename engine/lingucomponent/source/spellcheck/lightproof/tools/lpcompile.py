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
LPR_VERSION = 3

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
    "morph": (2, 4),
    "affix": (3, 4),
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
    """A construct this compiler does not lower yet.

    This fails the build: the rule is fine and the compiler needs work.
    """


class BrokenRule(Exception):
    """A rule that cannot run at all, in Python either.

    Conditions that name something undefined, reference a group their own
    pattern does not have, or apply a sign to a string raise as soon as they
    are reached, so the rule has never fired. They are dropped with a warning
    rather than failing the build.
    """


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
    tree = ast.parse(open(path, encoding="utf-8-sig").read())
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
                # ICU reads a nested set, a set intersection, a string in a
                # set and a POSIX class opener where Python re has four plain
                # characters.
                if pattern[j] in "[&{:":
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

    icu_pattern = "".join(out)
    check_icu_classes(icu_pattern)
    return icu_pattern, flags, names


def check_icu_classes(pattern):
    """Guards against an ICU set metacharacter left unescaped.

    ICU gives "[", "&", "{" and ":" meanings inside a character class that
    Python re does not, and a pattern that keeps one is rejected outright at
    run time, taking its rule with it. This has caught three separate cases,
    so it fails the build rather than relying on a warning nobody reads.
    """
    i = 0
    n = len(pattern)
    while i < n:
        if pattern[i] == "\\":
            i += 2
            continue
        if pattern[i] != "[":
            i += 1
            continue
        j = i + 1
        if j < n and pattern[j] == "^":
            j += 1
        first = True
        while j < n and pattern[j] != "]":
            if pattern[j] == "\\":
                j += 2
                first = False
                continue
            if pattern[j] in "[&{" or (pattern[j] == ":" and first):
                raise Unsupported("unescaped %r inside a character class" % pattern[j])
            first = False
            j += 1
        i = j + 1


def mandatory_literals(pattern):
    """The literal runs an ICU pattern must contain wherever it matches.

    Only text outside every group counts. Text inside one may sit in a branch
    of an alternation and so need not appear at all, and telling the two apart
    is not worth the risk: a literal wrongly called mandatory would make the
    runtime skip a rule that could have matched.
    """
    out = []
    run = []
    depth = 0
    i = 0
    n = len(pattern)

    def flush():
        text = "".join(run).strip().lower()
        if len(text) >= 3:
            out.append(text)
        run.clear()

    while i < n:
        c = pattern[i]
        following = pattern[i + 1] if i + 1 < n else ""
        if c == "\\":
            flush()
            i += 2
            continue
        if c == "[":
            flush()
            j = i + 1
            if j < n and pattern[j] == "^":
                j += 1
            if j < n and pattern[j] == "]":
                j += 1
            while j < n and pattern[j] != "]":
                j += 2 if pattern[j] == "\\" else 1
            i = j + 1
            continue
        if c == "(":
            flush()
            depth += 1
            i += 1
            continue
        if c == ")":
            flush()
            depth -= 1
            i += 1
            continue
        if depth > 0:
            i += 1
            continue
        if c in "|*+?{}^$.":
            # A quantifier makes the character before it optional.
            if c in "*?" and run:
                run.pop()
            flush()
            i += 1
            continue
        if following in "*?{":
            flush()
            i += 1
            continue
        run.append(c)
        i += 1

    flush()
    return out


def filter_key(pattern):
    """A rule's paragraph filter: a hash of its most selective literal.

    Zero means the rule has no literal to go on and always runs.
    """
    literals = mandatory_literals(pattern)
    if not literals:
        return 0
    longest = max(literals, key=len)
    # Only the first three characters are hashed, because that is what the
    # runtime can index a paragraph by in one pass.
    key = 0
    for character in longest[:3]:
        key = (key * 131 + ord(character)) & 0xFFFFFFFF
    return (key % 8191) + 1


# ---------------------------------------------------------------- lowering

class Lowerer:
    """Lowers a Python expression to .lpr bytecode.

    Only the subset the rule packages actually use is accepted; anything
    else raises Unsupported so the build fails loudly rather than shipping a
    rule that silently never fires.
    """

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
            raise BrokenRule("%s is not defined" % node.id)

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

    COMPARISONS = { ast.In: OP_IN, ast.NotIn: OP_NOT_IN, ast.Eq: OP_EQ, ast.NotEq: OP_NE }

    def v_Compare(self, node):
        # "a in b in c" means "(a in b) and (b in c)", which is what the few
        # rules that write it get.
        patches = []
        left = node.left
        for position, (op, right) in enumerate(zip(node.ops, node.comparators)):
            opcode = self.COMPARISONS.get(type(op))
            if opcode is None:
                raise Unsupported("comparison %s" % type(op).__name__)
            if position:
                self.emit(OP_POP)
            self.visit(left)
            self.visit(right)
            self.emit(opcode)
            if position < len(node.ops) - 1:
                patches.append(len(self.code))
                self.emit(OP_JMP_IF_FALSE_KEEP, 0)
            left = right
        for site in patches:
            struct.pack_into("<I", self.code, site + 1, len(self.code))

    def v_Subscript(self, node):
        if isinstance(node.slice, ast.Slice):
            if node.slice.step is not None:
                raise Unsupported("slice with a step")
            self.visit(node.value)
            self.bound(node.slice.lower)
            self.bound(node.slice.upper)
            self.emit(OP_SLICE)
            return
        self.visit(node.value)
        self.visit(node.slice)
        self.emit(OP_INDEX)

    def bound(self, node):
        """A slice bound, with null standing in for one the rule left out."""
        if node is None:
            self.emit(OP_PUSH_NULL)
        else:
            self.visit(node)

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
            raise BrokenRule("the pattern has no group named %s" % value)
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
                raise BrokenRule("a sign applied to a computed value")
            if not isinstance(value, int):
                raise BrokenRule("a sign applied to a string")
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

        if node.func.id == "suggest_foreign":
            # The Python closes over the module's phrase list, so it is
            # handed over explicitly here.
            if len(node.args) != 1 or "foreign" not in self.tables:
                raise Unsupported("suggest_foreign() without a phrase list")
            self.visit(node.args[0])
            self.emit(OP_PUSH_CONST, self.w.const_for_table("foreign", self.tables["foreign"]))
            self.code.append(OP_CALL)
            self.code.append(func_id)
            self.code.append(2)
            return

        if node.func.id == "calc":
            if len(node.args) != 2 or not isinstance(node.args[1], ast.Tuple):
                raise Unsupported("calc() without an argument tuple")
            self.visit(node.args[0])
            for element in node.args[1].elts:
                self.visit(element)
            self.code.append(OP_CALL)
            self.code.append(func_id)
            self.code.append(1 + len(node.args[1].elts))
            return
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
            if method not in ("group", "start", "end") or len(node.args) > 1:
                raise Unsupported("m.%s()" % method)
            opcode = { "group": OP_GROUP, "start": OP_MSTART, "end": OP_MEND }[method]
            self.emit(opcode, self.group_index(node.args[0]) if node.args else 0)
            return

        if method in self.STRING_METHODS:
            if node.args:
                raise Unsupported("%s() with arguments" % method)
            self.visit(receiver)
            self.emit(self.STRING_METHODS[method])
            return

        if method == "replace":
            if len(node.args) not in (2, 3):
                raise Unsupported("replace() with %d arguments" % len(node.args))
            self.visit(receiver)
            self.visit(node.args[0])
            self.visit(node.args[1])
            if len(node.args) == 3:
                self.visit(node.args[2])
            else:
                self.emit(OP_PUSH_NULL)
            self.emit(OP_REPLACE)
            return

        if method == "translate":
            # The rules only ever build the table inline, one character
            # mapped to one character.
            table = node.args[0] if len(node.args) == 1 else None
            if not (isinstance(table, ast.Call) and isinstance(table.func, ast.Attribute)
                    and table.func.attr == "maketrans" and len(table.args) == 2):
                raise Unsupported("translate() with a computed table")
            self.visit(receiver)
            self.visit(table.args[0])
            self.visit(table.args[1])
            self.emit(OP_TRANSLATE)
            return

        if isinstance(receiver, ast.Name) and receiver.id == "re":
            self.lower_re_call(method, node.args)
            return

        if method in self.REGEX_METHODS:
            if not isinstance(receiver, ast.Name):
                raise Unsupported("%s() on a computed pattern" % method)
            if receiver.id not in self.tables:
                raise BrokenRule("%s is not defined" % receiver.id)
            if len(node.args) != 1:
                raise Unsupported("%s() with %d arguments" % (method, len(node.args)))
            self.visit(node.args[0])
            self.emit(self.REGEX_METHODS[method],
                      self.w.const_for_table(receiver.id, self.tables[receiver.id]))
            return

        raise Unsupported("method .%s()" % method)

    def lower_re_call(self, method, args):
        """re.match/search/sub with the pattern written out in the rule."""
        opcodes = { "match": OP_RE_MATCH, "search": OP_RE_SEARCH, "sub": OP_RE_SUB }
        opcode = opcodes.get(method)
        if opcode is None:
            raise Unsupported("re.%s()" % method)
        expected = 3 if method == "sub" else 2
        if len(args) != expected:
            raise Unsupported("re.%s() with %d arguments" % (method, len(args)))
        try:
            pattern = ast.literal_eval(args[0])
        except ValueError:
            raise Unsupported("re.%s() with a computed pattern" % method)

        if method == "sub":
            self.visit(args[2])
            self.visit(args[1])
        else:
            self.visit(args[1])
        self.emit(opcode, self.w.const_regex(pattern))


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


def lower_rule(lower, writer, condition, replacement, message):
    """Lowers a rule's condition, replacement and message."""
    condition_code = 0
    if isinstance(condition, str) and condition.strip():
        condition_code = lower(condition, "condition")

    replacement_code = 0
    replacement_off = 0
    if isinstance(replacement, str) and replacement.startswith("="):
        replacement_code = lower(replacement[1:], "replacement")
    else:
        replacement_off = writer.string(replacement)

    message_code = 0
    message_off = 0
    if isinstance(message, str) and message.startswith("="):
        message_code = lower(message[1:], "message")
    else:
        message_off = writer.string(message)

    return condition_code, replacement_code, replacement_off, message_code, message_off


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
    broken = 0

    for index, rule in enumerate(rules):
        pattern, replacement, message, condition = rule[:4]
        if len(rule) > 4:
            ngroup = rule[4]
        else:
            # Brazilian Portuguese ships rules without the group field. The
            # Python then reads the case-sensitivity flag its rule compiler
            # appends in its place, so a case-insensitive rule marks group 1
            # and every other rule marks the whole match. Carried over as it
            # behaves; changing it is a rule-data change.
            ngroup = 1 if pattern.startswith("(?iu)") else 0
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
            except SyntaxError as error:
                raise BrokenRule("%s is not valid Python: %s" % (what, error))
            except Unsupported as error:
                sys.exit("lpcompile: %s rule %d: %s: %s" % (pkg, index, what, error))

        try:
            condition_code, replacement_code, replacement_off, message_code, message_off \
                = lower_rule(lower_or_die, writer, condition, replacement, message)
        except BrokenRule as error:
            sys.stderr.write("lpcompile: %s rule %d cannot run (%s), dropped\n"
                             % (pkg, index, error))
            broken += 1
            continue

        group_first = len(group_name_table)
        for name in sorted(names):
            group_name_table.append((writer.string(name), names[name]))

        rule_records.append((
            writer.string(icu_pattern), flags, filter_key(icu_pattern),
            replacement_off, replacement_code,
            message_off, message_code,
            condition_code, ngroup,
            group_first, len(names),
        ))

    if dropped or broken:
        sys.stderr.write("lpcompile: %s: %d rule(s) with a bad pattern, %d that cannot run\n"
                         % (pkg, dropped, broken))

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
    rules_off = place(b"".join(struct.pack("<11I", *r) for r in rule_records))
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
