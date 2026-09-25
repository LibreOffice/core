#!/usr/bin/env python3
"""Tests for the intro-document l10n tooling (l10n_odf.py and the two
scripts built on it).

The K* / M* / S* / X* cases below are the Python half of the shared parity
table; the C++ half lives in the engine tree at
desktop/qa/desktop_lib/test_l10ntranslate.cxx and must be kept in sync
case by case.  A change to the key spec must touch both files.
"""

import os
import subprocess
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from l10n_odf import (  # noqa: E402
    FORBIDDEN,
    MARKED,
    NOT_MARKED,
    TEXT_NS,
    analyze_document,
    build_key,
    collect_runs,
    derive_key_from_paragraph_xml,
    is_xml_safe_text,
    msgid_tag_count,
    parse_msgstr_tokens,
    parse_xml_bytes,
    rewrite_sheet_refs,
)

INTRODOCS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class KeyDerivationTest(unittest.TestCase):
    """Parity cases K1..K22 (keep in sync with test_l10ntranslate.cxx)."""

    CASES = [
        ("K1",
         '<text:p><text:span text:style-name="T5">_3. Press </text:span>'
         '<text:span text:style-name="T6">Enter</text:span>'
         '<text:span text:style-name="T7"> to calculate the total number of items</text:span>'
         '</text:p>',
         MARKED,
         "<1>3. Press </1><2>Enter</2><3> to calculate the total number of items</3>"),
        ("K2", '<text:p><text:span text:style-name="T1">_Hello</text:span></text:p>',
         MARKED, "Hello"),
        ("K3", "<text:p>_Plain paragraph text</text:p>", MARKED, "Plain paragraph text"),
        ("K4",
         '<text:p><text:span text:style-name="T1">_</text:span>'
         '<text:span text:style-name="T2">➜</text:span> Try it</text:p>',
         MARKED, "<1>➜</1> Try it"),
        ("K5", "<text:p>__foo</text:p>", MARKED, "_foo"),
        ("K6", "<text:p>Hello</text:p>", NOT_MARKED, ""),
        ("K7",
         '<text:p><text:span text:style-name="T1">Hello </text:span>'
         '<text:span text:style-name="T2">_world</text:span></text:p>',
         NOT_MARKED, ""),
        ("K8", "<text:p>_a<text:s/>b</text:p>", FORBIDDEN, ""),
        ("K9", "<text:p>_a<text:tab/>b</text:p>", FORBIDDEN, ""),
        ("K10", "<text:p>_a<text:line-break/>b</text:p>", FORBIDDEN, ""),
        ("K11", '<text:p>_a<text:a xlink:href="http://x/">link</text:a></text:p>',
         FORBIDDEN, ""),
        ("K12",
         '<text:p><text:span text:style-name="T1">_a'
         '<text:span text:style-name="T2">nested</text:span></text:span></text:p>',
         FORBIDDEN, ""),
        ("K13", "<text:p>_</text:p>", MARKED, ""),
        ("K14",
         '<text:p><text:span text:style-name="T1">_a</text:span>'
         '<text:span text:style-name="T2"></text:span>'
         '<text:span text:style-name="T3">b</text:span></text:p>',
         MARKED, "<1>a</1><2>b</2>"),
        ("K15", "<text:p></text:p>", NOT_MARKED, ""),
        ("K16", '<text:p>_one <text:span text:style-name="T1">two</text:span> three</text:p>',
         MARKED, "one <1>two</1> three"),
        ("K17", "<text:p>_hello<!-- x -->world</text:p>", MARKED, "helloworld"),
        ("K18",
         '<text:p>_a<!-- x -->b<text:span text:style-name="T1">c</text:span></text:p>',
         MARKED, "ab<1>c</1>"),
        ("K19", "<text:p>_hello<?pi x?>world</text:p>", MARKED, "helloworld"),
        ("K20", '<text:p>_a<text:span text:style-name="T2"/>b</text:p>', MARKED, "ab"),
        ("K21",
         '<text:p>_x<text:span text:style-name="T1">a<!-- c -->b</text:span></text:p>',
         MARKED, "x<1>ab</1>"),
        ("K22", "<text:p>_a<![CDATA[b]]>c</text:p>", MARKED, "abc"),
    ]

    def test_cases(self):
        for case_id, xml, expected_state, expected_key in self.CASES:
            state, key = derive_key_from_paragraph_xml(xml)
            self.assertEqual(expected_state, state, case_id)
            self.assertEqual(expected_key, key, case_id)


class RunMergeTest(unittest.TestCase):
    """R1..R4: invisible nodes (comments, PIs, empty spans) do not split
    bare text, so both implementations derive the same run list no matter
    how their XML parser splits text nodes (ElementTree folds CDATA and
    dropped comments into one text; libxml2 keeps the pieces separate).
    The budget mirror below must stay equal to msgid_tag_count() for
    every shape."""

    def merged_runs(self, snippet):
        wrapped = f'<root xmlns:text="{TEXT_NS}">{snippet}</root>'
        result = collect_runs(next(iter(parse_xml_bytes(wrapped.encode("utf-8")))))
        self.assertEqual(MARKED, result.state, snippet)
        return result.runs

    def test_cases(self):
        cases = [
            # R1: comment between bare texts
            ("<text:p>_hello<!-- x -->world</text:p>", [("helloworld", False)]),
            # R2: processing instruction between bare texts
            ("<text:p>_hello<?pi x?>world</text:p>", [("helloworld", False)]),
            # R3: empty span between bare texts
            ('<text:p>_a<text:span text:style-name="T2"/>b</text:p>', [("ab", False)]),
            # R4: merged bare text stays one untagged part next to a span
            ('<text:p>_a<!-- x -->b<text:span text:style-name="T1">c</text:span></text:p>',
             [("ab", False), ("c", True)]),
        ]
        for snippet, expected in cases:
            runs = self.merged_runs(snippet)
            self.assertEqual(expected, [(run.text, run.is_span) for run in runs], snippet)
            # the engine's run-derived budget (handleParagraph) must agree
            # with the msgid-derived one used by validate_translations
            budget = 0 if len(runs) == 1 else sum(run.is_span for run in runs)
            self.assertEqual(msgid_tag_count(build_key(runs)), budget, snippet)


class MsgstrTokenTest(unittest.TestCase):
    """Parity cases M1..M13 (keep in sync with test_l10ntranslate.cxx)."""

    def test_cases(self):
        # M1: straight order
        self.assertEqual([(1, "a"), (2, "b"), (3, "c")],
                         parse_msgstr_tokens("<1>a</1><2>b</2><3>c</3>", 3))
        # M2: reorder
        self.assertEqual([(2, "b"), (1, "a")], parse_msgstr_tokens("<2>b</2><1>a</1>", 2))
        # M3: repetition plus untagged literal
        self.assertEqual([(1, "a"), (0, " mitte "), (1, "b")],
                         parse_msgstr_tokens("<1>a</1> mitte <1>b</1>", 1))
        # M4: plain text only
        self.assertEqual([(0, "plain text")], parse_msgstr_tokens("plain text", 3))
        # M5: omission
        self.assertEqual([(1, "a"), (3, "c")], parse_msgstr_tokens("<1>a</1><3>c</3>", 3))
        # M6: unknown run index
        self.assertIsNone(parse_msgstr_tokens("<4>x</4>", 3))
        # M7: unclosed tag
        self.assertIsNone(parse_msgstr_tokens("<1>unclosed", 1))
        # M8: nesting
        self.assertIsNone(parse_msgstr_tokens("<1>a<2>b</2></1>", 2))
        # M9: stray close
        self.assertIsNone(parse_msgstr_tokens("a </2> b", 2))
        # M10: a lone "<" is literal text
        self.assertEqual([(0, "a < b > c")], parse_msgstr_tokens("a < b > c", 1))
        # M11: zero is not a valid run
        self.assertIsNone(parse_msgstr_tokens("<0>x</0>", 1))
        # M12: empty msgstr parses to zero tokens
        self.assertEqual([], parse_msgstr_tokens("", 1))
        # M13: a tagless msgid (tag_count 0) rejects any tag
        self.assertIsNone(parse_msgstr_tokens("<1>x</1>", 0))
        self.assertEqual([(0, "plain")], parse_msgstr_tokens("plain", 0))

    def test_tag_count(self):
        self.assertEqual(0, msgid_tag_count("plain"))
        self.assertEqual(3, msgid_tag_count("<1>a</1><2>b</2><3>c</3>"))
        self.assertEqual(1, msgid_tag_count("Visit <1>the Insert tab</1> now"))


class XmlSafeTextTest(unittest.TestCase):
    """Parity cases X1..X6 (keep in sync with test_l10ntranslate.cxx)."""

    def test_cases(self):
        # X1: ordinary text, accents and astral characters are all fine
        self.assertTrue(is_xml_safe_text("Druecken Sie Enter"))
        self.assertTrue(is_xml_safe_text("Einführung \U0001f44b"))
        # X2: the three control characters XML allows
        self.assertTrue(is_xml_safe_text("a\tb\nc\rd"))
        # X3: vertical tab and form feed are not among them
        self.assertFalse(is_xml_safe_text("Enter\x0bnow"))
        self.assertFalse(is_xml_safe_text("Enter\x0cnow"))
        # X4: NUL and the other C0 controls
        self.assertFalse(is_xml_safe_text("a\x00b"))
        self.assertFalse(is_xml_safe_text("a\x01b"))
        self.assertFalse(is_xml_safe_text("a\x1fb"))
        # X5: 0x7F-0x9F are legal in XML 1.0, and ODF is 1.0
        self.assertTrue(is_xml_safe_text("a\x7fb\x85c\x9fd"))
        # X6: the non-characters at the end of the BMP, and lone surrogates
        self.assertFalse(is_xml_safe_text("a\ufffeb"))
        self.assertFalse(is_xml_safe_text("a\uffffb"))
        self.assertTrue(is_xml_safe_text("a\ufffdb"))
        self.assertFalse(is_xml_safe_text("a\ud800b"))


class SheetRefRewriteTest(unittest.TestCase):
    """Parity cases S1..S5.  The engine asserts the same semantics
    end-to-end in its testTranslateFull; S5 (prefix ordering) is covered
    here only, guarded by the shared implementation comment."""

    def test_cases(self):
        # S1: plain quoted reference
        self.assertEqual("'\U0001f44b Einführung'.A1",
                         rewrite_sheet_refs("'_\U0001f44b Intro'.A1",
                                            [("_\U0001f44b Intro",
                                              "\U0001f44b Einführung")]))
        # S2: apostrophes escape by doubling, on both sides
        self.assertEqual("'Blatt''s'.B2:'Blatt''s'.C3",
                         rewrite_sheet_refs("'_A''s sheet'.B2:'_A''s sheet'.C3",
                                            [("_A's sheet", "Blatt's")]))
        # S3: bare reference, simple replacement name stays bare
        self.assertEqual("Daten.A1",
                         rewrite_sheet_refs("_Data.A1", [("_Data", "Daten")]))
        # S4: bare reference, replacement needing quotes gets them
        self.assertEqual("'Da ten'.A1",
                         rewrite_sheet_refs("_Data.A1", [("_Data", "Da ten")]))
        # S5: longest name first, so a prefix cannot corrupt the longer name
        self.assertEqual("'Y'.A1",
                         rewrite_sheet_refs("'_Data2'.A1",
                                            [("_Data", "X"), ("_Data2", "Y")]))


CONTENT_XML = """<?xml version="1.0" encoding="UTF-8"?>
<office:document-content
 xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:calcext="urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"
 office:version="1.3">
<office:automatic-styles>
<style:style style:name="cond1" style:family="table-cell">
<style:map style:base-cell-address="&apos;_Try it&apos;.A1"/>
</style:style>
</office:automatic-styles>
<office:body><office:spreadsheet>
<table:content-validations>
<table:content-validation table:name="val1"
 table:base-cell-address="&apos;_Try it&apos;.B2">
<table:help-message table:title="_Try formatting" table:display="true">
<text:p>_Explore the Format tab</text:p>
</table:help-message>
</table:content-validation>
</table:content-validations>
<table:table table:name="_Try it">
<table:table-column/>
<table:table-row>
<table:table-cell office:value-type="string">
<text:p><text:span text:style-name="T5">_3. Press </text:span><text:span
 text:style-name="T6">Enter</text:span><text:span
 text:style-name="T7"> to calculate</text:span></text:p>
</table:table-cell>
<table:table-cell office:value-type="string">
<text:p>_Visit <text:span text:style-name="T9">the Insert tab</text:span> now</text:p>
</table:table-cell>
<table:table-cell office:value-type="string">
<text:p><text:span text:style-name="T8">_Single span cell</text:span></text:p>
</table:table-cell>
</table:table-row>
<calcext:conditional-formats>
<calcext:conditional-format
 calcext:target-range-address="&apos;_Try it&apos;.B3:&apos;_Try it&apos;.B9"/>
</calcext:conditional-formats>
</table:table>
</office:spreadsheet></office:body></office:document-content>
"""

STYLES_XML = """<?xml version="1.0" encoding="UTF-8"?>
<office:document-styles
 xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0">
<office:master-styles><text:p>_Master text</text:p></office:master-styles>
</office:document-styles>
"""

MANIFEST_XML = """<?xml version="1.0" encoding="UTF-8"?>
<manifest:manifest
 xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
 manifest:version="1.3">
<manifest:file-entry manifest:full-path="/" manifest:version="1.3"
 manifest:media-type="application/vnd.oasis.opendocument.spreadsheet"/>
<manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml"/>
<manifest:file-entry manifest:full-path="styles.xml" manifest:media-type="text/xml"/>
</manifest:manifest>
"""


def write_fixture_ods(path, content_xml=CONTENT_XML, styles_xml=STYLES_XML):
    with zipfile.ZipFile(path, "w") as odf:
        mimetype = zipfile.ZipInfo("mimetype")
        odf.writestr(mimetype, "application/vnd.oasis.opendocument.spreadsheet")
        odf.writestr("META-INF/manifest.xml", MANIFEST_XML)
        odf.writestr("content.xml", content_xml)
        if styles_xml is not None:
            odf.writestr("styles.xml", styles_xml)


class AnalyzeDocumentTest(unittest.TestCase):
    def analyze_fixture(self, **kwargs):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "fixture.ods")
            write_fixture_ods(path, **kwargs)
            return analyze_document(path)

    def test_units_of_clean_fixture(self):
        units, errors, warnings = self.analyze_fixture()
        self.assertEqual([], errors)
        self.assertEqual([], warnings)
        by_key = {unit.key: unit.kind for unit in units}
        self.assertEqual(
            {
                "Try formatting": "help-title",
                "Explore the Format tab": "paragraph",
                "Try it": "sheet-name",
                "<1>3. Press </1><2>Enter</2><3> to calculate</3>": "paragraph",
                "Visit <1>the Insert tab</1> now": "paragraph",
                "Single span cell": "paragraph",
                "Master text": "paragraph",
            },
            by_key)

    def test_forbidden_content_is_an_error(self):
        _, errors, _ = self.analyze_fixture(styles_xml=STYLES_XML.replace(
            "<text:p>_Master text</text:p>",
            "<text:p>_Master<text:tab/>text</text:p>"))
        self.assertTrue(any("marked paragraph" in error for error in errors), errors)

    def test_whitespace_after_marker_is_an_error(self):
        _, errors, _ = self.analyze_fixture(styles_xml=STYLES_XML.replace(
            "_Master text", "_ Master text"))
        self.assertTrue(any("whitespace directly after" in error for error in errors),
                        errors)

    def test_underscore_mid_paragraph_is_text_not_an_error(self):
        # "_" is a marker only as the paragraph's first character; anywhere
        # else it is literal text - mentioned as a warning, never an error
        units, errors, warnings = self.analyze_fixture(styles_xml=STYLES_XML.replace(
            "<text:p>_Master text</text:p>",
            '<text:p>Master <text:span text:style-name="T1">_text</text:span></text:p>'))
        self.assertEqual([], errors)
        self.assertTrue(any("non-first run" in warning for warning in warnings),
                        warnings)
        self.assertNotIn("Master", [unit.key for unit in units])  # not a unit

    def test_underscore_in_untranslated_attribute_is_text_not_an_error(self):
        _, errors, warnings = self.analyze_fixture(content_xml=CONTENT_XML.replace(
            'table:name="val1"', 'table:name="_val1"'))
        self.assertEqual([], errors)
        self.assertTrue(any("untranslated attribute" in warning for warning in warnings),
                        warnings)

    def test_formula_with_marked_sheets_is_an_error(self):
        _, errors, _ = self.analyze_fixture(content_xml=CONTENT_XML.replace(
            '<table:table-cell office:value-type="string">',
            '<table:table-cell table:formula="of:=[.A1]" office:value-type="string">'))
        self.assertTrue(any("table:formula" in error for error in errors), errors)

    def test_unquoted_sheet_reference_is_an_error(self):
        _, errors, _ = self.analyze_fixture(content_xml=CONTENT_XML.replace(
            "&apos;_Try it&apos;.B2", "_Try it.B2"))
        self.assertTrue(any("unquoted reference" in error for error in errors), errors)


class ScriptEndToEndTest(unittest.TestCase):
    """Drive the two scripts the way the build does."""

    def run_script(self, script, *args):
        # capture_output= and text= need Python 3.7; the build runs this on 3.6 too
        return subprocess.run(
            [sys.executable, os.path.join(INTRODOCS_DIR, script), *args],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True, cwd=INTRODOCS_DIR)

    def test_extract_check_and_embed(self):
        with tempfile.TemporaryDirectory() as tmp:
            ods = os.path.join(tmp, "Fixture-Template.ods")
            pot = os.path.join(tmp, "Fixture-Template.pot")
            write_fixture_ods(ods)

            # extract writes a pot, and --check right after it passes
            result = self.run_script("extract_odf_text.py", ods, pot)
            self.assertEqual(0, result.returncode, result.stderr)
            result = self.run_script("extract_odf_text.py", "--check", ods, pot)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            # another polib version orders the header fields differently
            with open(pot, encoding="utf-8") as pot_file:
                reordered = pot_file.read().replace(
                    '"Language: \\n"\n', "").replace(
                    '"Content-Transfer-Encoding: 8bit\\n"\n',
                    '"Content-Transfer-Encoding: 8bit\\n"\n"Language: \\n"\n')
            with open(pot, "w", encoding="utf-8") as pot_file:
                pot_file.write(reordered)
            result = self.run_script("extract_odf_text.py", "--check", ods, pot)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            # --check flags a stale pot with exit code 2 and a diff
            with open(pot, encoding="utf-8") as pot_file:
                stale = pot_file.read().replace("Enter", "Escape")
            with open(pot, "w", encoding="utf-8") as pot_file:
                pot_file.write(stale)
            result = self.run_script("extract_odf_text.py", "--check", ods, pot)
            self.assertEqual(2, result.returncode)
            self.assertIn("stale", result.stderr)

            # a translated po gets embedded with verbatim keys; a fuzzy
            # entry, a tag-mismatched one and one holding a character
            # illegal in XML are dropped
            self.run_script("extract_odf_text.py", ods, pot)
            po_path = os.path.join(tmp, "Fixture-Template-de.po")
            with open(pot, encoding="utf-8") as pot_file:
                po_text = pot_file.read()
            po_text = po_text.replace(
                'msgid "Try it"\nmsgstr ""',
                'msgid "Try it"\nmsgstr "Probier\'s"')
            po_text = po_text.replace(
                'msgid "<1>3. Press </1><2>Enter</2><3> to calculate</3>"\nmsgstr ""',
                'msgid "<1>3. Press </1><2>Enter</2><3> to calculate</3>"\n'
                'msgstr "<2>Enter</2><1>3. Drücken Sie </1><3> zum Rechnen</3>"')
            po_text = po_text.replace(
                'msgid "Master text"\nmsgstr ""',
                'msgid "Master text"\nmsgstr "<9>bad</9>"')
            po_text = po_text.replace(
                'msgid "Try formatting"\nmsgstr ""',
                '#, fuzzy\nmsgid "Try formatting"\nmsgstr "Unsicher"')
            # a vertical tab, as a paste into a translation editor leaves it
            po_text = po_text.replace(
                'msgid "Explore the Format tab"\nmsgstr ""',
                'msgid "Explore the Format tab"\n'
                'msgstr "Erkunde\x0bden Format-Tab"')
            with open(po_path, "w", encoding="utf-8") as po_file:
                po_file.write(po_text)

            result = self.run_script("create_l10n_intro_docs.py", tmp, ods)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

            with zipfile.ZipFile(ods) as odf:
                stream = odf.read("l10n").decode("utf-8")
                manifest = odf.read("META-INF/manifest.xml").decode("utf-8")
                # the l10n templates are exact copies of the marked members
                self.assertEqual(odf.read("content.xml"),
                                 odf.read("l10n_template.xml"))
                self.assertEqual(odf.read("styles.xml"),
                                 odf.read("l10n_template_styles.xml"))
            self.assertTrue(stream.startswith("# supported locales\n"), stream)
            self.assertIn("\nde\n", stream)  # locale list
            self.assertIn("\nTry it\nde\tProbier's\n", stream)
            self.assertIn(
                "\n<1>3. Press </1><2>Enter</2><3> to calculate</3>\n"
                "de\t<2>Enter</2><1>3. Drücken Sie </1><3> zum Rechnen</3>\n",
                stream)
            self.assertNotIn("Unsicher", stream)  # fuzzy dropped
            self.assertNotIn("bad", stream)  # tag mismatch dropped
            self.assertNotIn("Erkunde", stream)  # illegal XML character dropped
            self.assertIn("illegal in XML", result.stderr)
            self.assertNotIn("_", stream.split("# strings")[1])  # keys carry no marker
            self.assertIn('manifest:full-path="l10n"', manifest)
            self.assertIn('manifest:full-path="l10n_template.xml"', manifest)
            self.assertIn('manifest:full-path="l10n_template_styles.xml"', manifest)

    def test_embed_without_po_files_writes_placeholder_locale(self):
        with tempfile.TemporaryDirectory() as tmp:
            ods = os.path.join(tmp, "Fixture-Template.ods")
            write_fixture_ods(ods)
            result = self.run_script("create_l10n_intro_docs.py", tmp, ods)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            with zipfile.ZipFile(ods) as odf:
                stream = odf.read("l10n").decode("utf-8")
                names = odf.namelist()
            self.assertIn("\n-\n", stream)  # placeholder locale list
            self.assertEqual("mimetype", names[0])  # member order preserved
            self.assertIn("l10n_template.xml", names)  # templates embedded anyway

    def test_extract_rejects_lint_errors(self):
        with tempfile.TemporaryDirectory() as tmp:
            ods = os.path.join(tmp, "Fixture-Template.ods")
            pot = os.path.join(tmp, "Fixture-Template.pot")
            write_fixture_ods(ods, styles_xml=STYLES_XML.replace(
                "_Master text", "_ Master text"))
            result = self.run_script("extract_odf_text.py", ods, pot)
            self.assertEqual(1, result.returncode)
            self.assertFalse(os.path.exists(pot))


if __name__ == "__main__":
    unittest.main()
