using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// The two padding directives an ODF number format states as something other than itself:
/// <c>_x</c>, which has no ODF spelling at all, and <c>*x</c>, which is an element.
/// </summary>
/// <remarks>
/// <para>
/// <c>_x</c> means "leave the width of <c>x</c> blank". ODF has no directive for it, so
/// LibreOffice's exporter writes the <em>spaces</em> into a <c>number:text</c> and records what
/// they stood for in <c>loext:blank-width-char</c> — a sequence of <c>&lt;char&gt;[position]</c>
/// groups separated by <c>_</c>, the positions counted in the unquoted text. <c>*x</c> becomes a
/// <c>number:fill-character</c> holding the character. Neither was read: the first became the
/// literal space the element carries and the second nothing at all, so an accounting format's
/// currency symbol ran into its digits and its columns did not line up.
/// </para>
/// <para>
/// <strong>Every expectation here is 26.2.4.2's own assembled code</strong>, read out of its HTML
/// export of this same file, where <c>sdnum</c>'s third field is the string
/// <c>SvNumberFormatter</c> holds. The rewrite is <c>lcl_InsertBlankWidthChars</c>
/// (<c>xmloff/source/style/xmlnumfi.cxx</c>:879-926) followed exactly, and the width of one blank
/// is <c>SvNumberformat::InsertBlanks</c>' <c>cCharWidths</c>
/// (<c>svl/source/numbers/zformat.cxx</c>:71-105).
/// </para>
/// <para>
/// <strong>The euro arm is the one that separates a faithful implementation from a plausible
/// one.</strong> <c>€</c> is above ASCII, so a blank standing in for it is <b>two</b> spaces and
/// not one, and its trailing text carries a multi-group spec with positions — <c>€1_-3</c> over
/// four spaces. A reader that removes one character per group, or that ignores the positions,
/// puts the directive in the wrong place and the error is invisible in the drawn characters.
/// </para>
/// </remarks>
public class OdfNumberFormatPaddingTests
{
    private const string Fixture = "sheet-odf-numfmt-padding.ods";
    private const string N = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
    private const string S = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";

    /// <summary>Every <c>number:*-style</c> in the fixture, by name, read from the file itself.</summary>
    /// <remarks>
    /// Read rather than transcribed, because the point of the fixture is that it is what
    /// 26.2.4.2's exporter wrote — a hand-copied fragment would be a claim about the transcription.
    /// </remarks>
    private static Dictionary<string, XElement> Styles()
    {
        Dictionary<string, XElement> styles = [];

        using ZipArchive archive = ZipFile.OpenRead(Corpus.Require(Fixture));
        foreach (string part in (string[])["styles.xml", "content.xml"])
        {
            if (archive.GetEntry(part) is not { } entry) continue;

            using Stream stream = entry.Open();
            foreach (XElement element in XDocument.Load(stream, LoadOptions.PreserveWhitespace)
                         .Descendants())
            {
                if (element.Name.NamespaceName != N) continue;
                if (!element.Name.LocalName.EndsWith("-style", StringComparison.Ordinal)) continue;
                if (element.Attribute(XName.Get("name", S))?.Value is { Length: > 0 } name)
                {
                    styles[name] = element;
                }
            }
        }

        return styles;
    }

    private static string? Code(string name)
    {
        Dictionary<string, XElement> styles = Styles();
        return OdfNumberFormat.Code(styles[name], mapped => styles.GetValueOrDefault(mapped));
    }

    [Theory]
    // The ASCII accounting format: one-character blanks, and a position on the one text that is
    // two characters long.
    [InlineData("N144", """[>0]_(* #,##0_);[<0]_(* (#,##0);_(* "-"_);_(@_)""")]
    // The euro accounting format: a two-space blank for `€`, and a multi-group spec with
    // positions on the four-space trailing text.
    [InlineData("N152", """[>0]_-* #,##0.00" "_€_-;[<0]-* #,##0.00" "_€_-;_-* -??" "_€_-;_-@_-""")]
    // A fill character with no blank beside it, so the two are separable.
    [InlineData("N153", "* #,##0")]
    // Neither, and the control.
    [InlineData("N2", "0.00")]
    public void TheAssembledCodeIsTheReferencesOwn(string name, string expected)
        => Code(name).ShouldBe(expected);
}
