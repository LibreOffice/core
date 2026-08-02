using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Widow and orphan control is on in a DOC unless the document turns it off.
/// </summary>
/// <remarks>
/// <para>
/// A binary Word file states a property only where it differs from a built-in default, so the defaults are
/// part of the format rather than a fallback a reader may choose. <c>WW8RStyle::Set1StyleDefaults</c>
/// (<c>sw/source/filter/ww8/ww8par2.cxx</c>:3751) puts <c>SvxWidowsItem(2)</c> and <c>SvxOrphansItem(2)</c>
/// on every root paragraph style that did not state <c>sprmPFWidowControl</c> itself, and the styles below
/// it inherit — so a document that never mentions the flag has it on everywhere.
/// </para>
/// <para>
/// Verified against the corpus rather than asserted from the source:
/// <c>words/batch-001/doc/advert-for-draft-decision-airbus-epr-bm3965ia-v006.doc</c> has a <c>Normal</c>
/// style carrying no sprms at all, and LibreOffice's flat-ODF export of it writes <c>fo:widows="2"</c> on
/// <c>Standard</c>. Reading the silence as "off" let a three-line paragraph split two-and-one across the
/// page break where the reference moves the whole paragraph onto the next page.
/// </para>
/// <para>
/// RTF shares <see cref="Ww8LayoutFormat.ToParagraphFormat"/> and is <em>not</em> affected: its reader
/// always states the flag, so the value is never absent there and the default never applies.
/// </para>
/// </remarks>
public sealed class DocWidowControlTests
{
    [Fact]
    public void ADocumentThatNeverMentionsWidowControlHasIt()
    {
        ParagraphFormat format = default(Ww8LayoutFormat).ToParagraphFormat(Length.FromPoints(11));

        format.WidowLines.ShouldBe(2);
        format.OrphanLines.ShouldBe(2);
    }

    [Fact]
    public void ADocumentThatTurnsWidowControlOffKeepsItOff()
    {
        ParagraphFormat format = new Ww8LayoutFormat { HasWidowControl = false }
            .ToParagraphFormat(Length.FromPoints(11));

        format.WidowLines.ShouldBe(0);
        format.OrphanLines.ShouldBe(0);
    }
}
