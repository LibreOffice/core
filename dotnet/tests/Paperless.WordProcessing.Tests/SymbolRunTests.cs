using Paperless.Core.Documents;
using Paperless.Text.Fonts;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The two run-level elements that carry a character and that only extraction had a case for.
/// </summary>
/// <remarks>
/// <para>
/// A <c>w:sym</c> and a <c>w:noBreakHyphen</c> are elements rather than characters, and the DOCX
/// <em>layout</em> walker's <c>default:</c> recursed into them, found no children and emitted
/// nothing — so the glyph was never drawn and the space it takes was closed up. Sixteen corpus
/// documents state 158 symbols between them and nine state 138 hyphens, one of them 105.
/// </para>
/// <para>
/// <c>symbol-run.docx</c> is <c>prose-docx.docx</c> with one paragraph added, so it carries the
/// styles, settings and theme parts a real writer emits — a hand-built DOCX without them lays out
/// in the application's fallback face and answers a different question. The paragraph reads
/// <c>alpha × beta ✔ gamma-600</c>, the two symbols stated as Symbol's <c>F0B4</c> and Wingdings'
/// <c>F0FC</c> and the hyphen as <c>w:noBreakHyphen</c>.
/// </para>
/// <para>
/// <strong>There is no <c>.doc</c> half of this fixture, and that is a property of the exporter
/// rather than an omission.</strong> LibreOffice's own MS Word 97 export writes the symbol as
/// private-use text against a symbol font and never as <c>sprmCSymbol</c> — the sprm's identifier
/// bytes do not occur anywhere in the converted file — so no fixture generated this way can
/// exercise the WW8 path. That claim rests on the corpus instead: nine <c>.doc</c> whose reference
/// embeds OpenSymbol where we embedded none, and whose text layers read <c>(</c> where the
/// reference reads the symbol.
/// </para>
/// </remarks>
public sealed class SymbolRunTests
{
    /// <summary>
    /// Both symbols are drawn, recoded, and set in the face that can actually show them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference's own PDF of this file draws <c>U+F0B4</c> and <c>U+F0FC</c> and embeds
    /// OpenSymbol for both; ours draws OpenSymbol's <c>U+E129</c> and <c>U+E4C2</c>, which are the
    /// same two pictures. The code points differ because LibreOffice writes the <em>original</em>
    /// slot into its <c>ToUnicode</c> while drawing the substitute's glyph, which
    /// <see cref="SymbolFontRecode"/> documents; both are private-use code points that mean nothing
    /// outside one font, so nothing a caller extracts depends on which.
    /// </para>
    /// <para>
    /// The face is asserted beside the character because the two are one decision. Asking OpenSymbol
    /// for <c>U+F0B4</c> is <c>.notdef</c> — its whole <c>F000</c>–<c>F0FF</c> coverage is ten code
    /// points — and asking the paragraph's Liberation Mono for <c>U+E129</c> is <c>.notdef</c> too.
    /// </para>
    /// <para>
    /// Reintroducing the bug to check this fails: delete the <c>case "sym"</c> from
    /// <c>DocxLayoutSource.RunWalker.Append</c>. The paragraph then reads <c>alpha  beta  gamma-600</c>
    /// and the count assertion fails at nought.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASymbolRunIsDrawnFromTheFaceThatCanShowIt()
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require("symbol-run.docx")));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageParagraph paragraph = pages.Paragraphs.First(p => p.Text.StartsWith("alpha", StringComparison.Ordinal));

        // Symbol's 0xB4 and Wingdings' 0xFC, through the tables ConvertChar::RecodeChar uses.
        paragraph.Text.ShouldBe("alpha  beta  gamma-600");

        List<PageRun> symbols =
        [
            .. paragraph.Runs.Where(run =>
                paragraph.Text.AsSpan(run.Start, run.Length).IndexOfAny('', '') >= 0),
        ];

        symbols.Count.ShouldBe(2);

        foreach (PageRun run in symbols)
        {
            SymbolFontRecode.IsSubstituteFamily(run.Font?.FamilyName).ShouldBeTrue(
                $"a recoded symbol was left in {run.Font?.FamilyName ?? "no face at all"}");
        }
    }

    /// <summary>
    /// A <c>w:noBreakHyphen</c> is drawn as an ordinary hyphen, not as U+2011.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The importer carries it as U+2011 (<c>OOXMLFastContextHandler.cxx:54</c>) and the layout then
    /// swaps the character out — <c>case CHAR_HARDHYPHEN: pPor = new SwBlankPortion('-')</c>,
    /// <c>sw/source/core/text/itrform2.cxx:1881-1882</c>. Both reference PDFs agree: this fixture's
    /// reads <c>gamma-600</c> with a U+002D, and so does the corpus's
    /// <c>Company-profile-2022-EN.docx</c>.
    /// </para>
    /// <para>
    /// It matters beyond the code point. U+2011 is in neither Carlito nor any Liberation face, so
    /// keeping it draws a fallback face's glyph in the middle of a word — measured on the <c>.doc</c>
    /// path, which still does that and pulls DejaVu Sans into the PDF for one character.
    /// </para>
    /// <para>
    /// What this deliberately does not assert is that the hyphen refuses to break: U+002D is UAX #14
    /// class HY and a <c>SwBlankPortion</c> is unbreakable, so that half is not reproduced and a test
    /// claiming it would fail.
    /// </para>
    /// <para>
    /// Reintroducing the bug to check this fails: delete the <c>case "noBreakHyphen"</c> from
    /// <c>DocxLayoutSource.RunWalker.Append</c> and the text reads <c>gamma600</c>; change its
    /// <c>Emit("-")</c> to <c>Emit("‑")</c> and it reads <c>gamma‑600</c>. Both fail here.
    /// </para>
    /// </remarks>
    [Fact]
    public void ANonBreakingHyphenIsDrawnAsAnOrdinaryHyphen()
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require("symbol-run.docx")));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageParagraph paragraph = pages.Paragraphs.First(p => p.Text.StartsWith("alpha", StringComparison.Ordinal));

        paragraph.Text.ShouldEndWith("gamma-600");
        paragraph.Text.ShouldNotContain('‑');
    }
}
