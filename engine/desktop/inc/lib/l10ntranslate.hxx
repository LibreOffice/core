/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <desktop/dllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <map>
#include <string_view>
#include <vector>

/* Translation of "_"-marked ODF templates carrying an "l10n" stream.
 *
 * The stream (zip member "l10n") maps whole-paragraph keys to translations
 * per locale.  Keys are derived by rules shared verbatim with
 * browser/introdocs/l10n_odf.py in online; the parity case table lives in
 * the unit tests on both sides.  translateDocument() is the entry point;
 * the helpers below are its testable building blocks.
 */
namespace desktop::l10n
{
enum class TranslateResult
{
    Translated, ///< l10n stream present and the requested language matched
    /// no stream (byte copy); no locale match or unreadable stream (markers stripped)
    CopiedUntranslated,
    /// the file could not be read, parsed as a package, rewritten, or written back
    Error,
};

/** Translate the ODF file at rInputPathOrUrl into rBcp47Language, writing
    rOutputPathOrUrl.  The paths may name the same file: the rewrite is then
    in place, atomic, and skipped when the result would be byte-identical.

    - No "l10n" zip member: byte-identical copy (a no-op in place).
    - Locale matched: marked paragraphs, sheet names and validation
      help-message titles are rewritten with the translations; every "_"
      marker is stripped.
    - No locale match, an empty rBcp47Language, or a stream that cannot be
      read: only the markers are stripped.
    - Translated before and edited since: the file is left exactly as it is
      and reported as CopiedUntranslated, because the change is the user's
      own work.

    content.xml and styles.xml are rewritten from their marked sources: the
    embedded l10n templates "l10n_template.xml" and
    "l10n_template_styles.xml" when present and readable, the members
    themselves otherwise.  All other members - the stream and templates
    included - are preserved byte-for-byte, in order, so "mimetype" stays
    first and STORED and the file can be translated again on a later open;
    the first user save rewrites the package and drops the machinery.

    A save that keeps the machinery instead is what the edited-file case
    above guards.  The stream carries an "# applied: <locale>" record of the
    locale the last rewrite used, and content.xml is rebuilt only while it
    still matches what that locale produced. */
DESKTOP_DLLPUBLIC TranslateResult translateDocument(const OUString& rInputPathOrUrl,
                                                    const OUString& rOutputPathOrUrl,
                                                    const OUString& rBcp47Language);

/// How a text:p / text:h element relates to the "_"-marker mechanism.
enum class UnitState
{
    NotMarked, ///< first run does not start with "_" (or there is no text)
    Marked, ///< a translation unit; a key was derived
    Forbidden, ///< contains content the spec bans inside marked paragraphs
};

/** Derive the translation key from a paragraph given as a standalone XML
    snippet (namespace prefixes "text" and "xlink" are predeclared).

    Runs are one per non-empty text:span child and one per stretch of bare
    text; a node that renders nothing (a comment, a PI, an empty span) does
    not split bare text.  Empty text contributes no run.  Exactly one
    leading "_" is stripped from the first run, dropping it if it becomes
    empty.  One run yields the bare text as
    key; with two or more runs, each span is wrapped in span order as
    &lt;1&gt;..&lt;/1&gt;&lt;2&gt;..&lt;/2&gt; while bare text stays untagged.
    A paragraph that is only "_" yields Marked with an empty key.  rKey is
    UTF-8. */
DESKTOP_DLLPUBLIC UnitState deriveKeyFromParagraphXml(std::string_view rParagraphXml,
                                                      OString& rKey);

/// One piece of a parsed msgstr: literal text, or text for span nRun.
struct MsgstrToken
{
    /// 0 = untagged literal: bare paragraph text, or the run's own formatting
    /// in a single-run paragraph.  Otherwise the 1-based span index.
    sal_Int32 nRun;
    OString aText;
};

/** Parse a msgstr into a flat token stream of literals and
    &lt;N&gt;text&lt;/N&gt; groups.  Tags may be reordered, repeated or
    omitted; nesting is invalid.  Returns false (and clears rTokens) on
    unknown or unclosed tags or N outside [1, nTagCount]. */
DESKTOP_DLLPUBLIC bool parseMsgstrTokens(const OString& rMsgstr, sal_Int32 nTagCount,
                                         std::vector<MsgstrToken>& rTokens);

/** True when rText is valid UTF-8 and holds only characters XML 1.0 allows
    (its Char production).

    The XML serializer escapes "&", "<" and ">" and nothing else: it
    range-checks no character.  A control character in a translation would
    therefore land in content.xml verbatim and the package would stop
    parsing, and invalid UTF-8 would be silently mangled into a numeric
    reference for the wrong character.  Translations come from .po files we
    do not control, so parseL10nStream() checks them on the way in.

    0x7F-0x9F stay allowed: only XML 1.1 restricts them, and ODF is 1.0.

    Mirrors is_xml_safe_text() in browser/introdocs/l10n_odf.py. */
DESKTOP_DLLPUBLIC bool isXmlSafeUtf8(const OString& rText);

/// Parsed "l10n" stream, reduced to the locale chosen for one request.
struct L10nStream
{
    OString aMatchedLocale; ///< empty when no locale in the stream matched
    std::map<OString, OString> aMap; ///< key (msgid) to translation, UTF-8
};

/** Parse the stream text: "#" comments, then a comma-separated locale list,
    then blocks of key line + "locale\tvalue" lines separated by blank lines.
    The chosen locale is the first of rBcp47Language's fallback strings, from
    the most specific tag to the least, that the stream's locale list holds,
    so a "pt-BR" reader gets the "pt-BR" translation even where the stream
    lists plain "pt" first.

    An empty rBcp47Language matches no locale: it leaves the map empty so
    that only the markers come off, rather than standing for the system
    locale the way LanguageTag reads it.

    A value that is not valid UTF-8, or that holds a character XML 1.0
    forbids, never enters the map: it is dropped with a warning, so the key
    misses and its marker is merely stripped.  Writing such a value out would
    produce a content.xml that no longer parses. */
DESKTOP_DLLPUBLIC void parseL10nStream(const OString& rStreamText, const OUString& rBcp47Language,
                                       L10nStream& rStream);

// Minimal zip access, exposed so tests can build fixtures and inspect output.
namespace zip
{
struct Entry
{
    OString aName;
    sal_uInt16 nMethod = 0; ///< 0 = stored, 8 = deflate
    std::vector<sal_uInt8> aData; ///< uncompressed content
    std::vector<sal_uInt8> aRawData; ///< compressed payload as stored (read only)
};

/// Read all entries, decompressed, in central-directory order.
DESKTOP_DLLPUBLIC bool readArchive(const OUString& rPathOrUrl, std::vector<Entry>& rEntries);

/// Write a fresh zip with the given entries in order, honouring nMethod.
DESKTOP_DLLPUBLIC bool writeArchive(const OUString& rPathOrUrl, const std::vector<Entry>& rEntries);
}

} // namespace desktop::l10n

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
