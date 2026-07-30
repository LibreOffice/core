using System.Xml.Linq;
using Paperless.Core.Documents;
using Paperless.Core.Formats;
using Paperless.OpenDocument;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>Tests for reading <c>meta.xml</c>.</summary>
public class OdfMetadataTests
{
    private static XElement Meta() => XElement.Parse($$"""
        <office:meta xmlns:office="{{OdfNamespaces.Office}}"
                     xmlns:meta="{{OdfNamespaces.Meta}}"
                     xmlns:dc="{{OdfNamespaces.DublinCore}}">
          <dc:title>A title</dc:title>
          <dc:subject>A subject</dc:subject>
          <dc:description>A description</dc:description>
          <meta:initial-creator>Ada Lovelace</meta:initial-creator>
          <dc:creator>Grace Hopper</dc:creator>
          <meta:creation-date>2026-01-02T03:04:05</meta:creation-date>
          <dc:date>2026-02-03T04:05:06</dc:date>
          <meta:print-date>2026-03-04T05:06:07</meta:print-date>
          <meta:keyword>alpha</meta:keyword>
          <meta:keyword>beta</meta:keyword>
          <meta:keyword>   </meta:keyword>
          <dc:language>en-GB</dc:language>
          <meta:generator>LibreOffice/24.2</meta:generator>
          <meta:editing-cycles>7</meta:editing-cycles>
          <meta:editing-duration>PT1H23M45S</meta:editing-duration>
          <meta:document-statistic meta:page-count="3" meta:word-count="146"
                                   meta:character-count="797" meta:paragraph-count="31"
                                   meta:table-count="2" meta:image-count="1"/>
          <meta:user-defined meta:name="Reviewer">Alan Turing</meta:user-defined>
          <meta:user-defined meta:name="Revision" meta:value-type="float">2.5</meta:user-defined>
          <meta:user-defined meta:name="Approved" meta:value-type="boolean">true</meta:user-defined>
          <meta:user-defined meta:name="Broken" meta:value-type="float">not a number</meta:user-defined>
          <dc:coverage></dc:coverage>
        </office:meta>
        """);

    [Fact]
    public void TheAuthorAndTheLastEditorAreNotConfused()
    {
        DocumentMetadata metadata = OdfMetadata.Read(Meta());

        // meta:initial-creator is the author; dc:creator is whoever saved it last. Reading
        // dc:creator as "the author" is the obvious mistake and this is what catches it.
        metadata.Author.ShouldBe("Ada Lovelace");
        metadata.LastModifiedBy.ShouldBe("Grace Hopper");
    }

    [Fact]
    public void CreationAndModificationDatesComeFromDifferentVocabularies()
    {
        DocumentMetadata metadata = OdfMetadata.Read(Meta());

        metadata.Created!.Value.Year.ShouldBe(2026);
        metadata.Created!.Value.Month.ShouldBe(1);
        metadata.Modified!.Value.Month.ShouldBe(2);
        metadata.Printed!.Value.Month.ShouldBe(3);
    }

    [Fact]
    public void SimpleFieldsAreRead()
    {
        DocumentMetadata metadata = OdfMetadata.Read(Meta());

        metadata.Title.ShouldBe("A title");
        metadata.Subject.ShouldBe("A subject");
        metadata.Description.ShouldBe("A description");
        metadata.Language.ShouldBe("en-GB");
        metadata.GeneratorApplication.ShouldBe("LibreOffice/24.2");
        metadata.RevisionNumber.ShouldBe(7);
        metadata.TotalEditingTime.ShouldBe(new TimeSpan(1, 23, 45));
    }

    [Fact]
    public void KeywordsAreOneElementEachAndBlanksAreDropped()
    {
        // Unlike OOXML's single delimited string there is nothing to split, so the only
        // question is whether an empty element becomes an empty keyword. It should not.
        OdfMetadata.Read(Meta()).Keywords.ShouldBe(["alpha", "beta"]);
    }

    [Fact]
    public void AnElementWrittenEmptyCountsAsNotRecorded()
    {
        // DocumentMetadata's contract is that null means "not recorded", and ODF writers
        // routinely emit an empty element rather than omitting it.
        OdfMetadata.Read(Meta()).CustomProperties.ShouldNotContainKey("coverage");
        OdfMetadata.Read(Meta()).Category.ShouldBeNull();
    }

    [Fact]
    public void UserDefinedPropertiesTakeTheirDeclaredType()
    {
        IReadOnlyDictionary<string, object?> properties = OdfMetadata.Read(Meta()).CustomProperties;

        properties["Reviewer"].ShouldBe("Alan Turing");
        properties["Revision"].ShouldBe(2.5);
        properties["Approved"].ShouldBe(true);
        // A value that does not parse as its declared type keeps its text: the name and what
        // the user typed are still the information they were after.
        properties["Broken"].ShouldBe("not a number");
    }

    [Fact]
    public void StatisticsAreInterpretedAccordingToTheFamily()
    {
        // ODF reuses the same counters across families: table-count is a spreadsheet's sheet
        // count, and page-count is a presentation's slide count.
        DocumentStatistics text = OdfMetadata.Read(Meta(), DocumentFamily.WordProcessing).Statistics!;
        text.PageCount.ShouldBe(3);
        text.TableCount.ShouldBe(2);
        text.SheetCount.ShouldBeNull();
        text.SlideCount.ShouldBeNull();
        text.WordCount.ShouldBe(146);
        text.CharacterCount.ShouldBe(797);
        text.ParagraphCount.ShouldBe(31);
        text.ImageCount.ShouldBe(1);

        DocumentStatistics sheet = OdfMetadata.Read(Meta(), DocumentFamily.Spreadsheet).Statistics!;
        sheet.SheetCount.ShouldBe(2);
        sheet.TableCount.ShouldBeNull();

        DocumentStatistics deck = OdfMetadata.Read(Meta(), DocumentFamily.Presentation).Statistics!;
        deck.SlideCount.ShouldBe(3);
        deck.PageCount.ShouldBeNull();
    }

    [Fact]
    public void NoMetadataAtAllIsNotAnError()
    {
        // Metadata is optional in ODF, so an absent office:meta must not throw.
        OdfMetadata.Read(null).ShouldBe(DocumentMetadata.Empty);
    }

    [Fact]
    public void AMetaElementWithNoChildrenYieldsNoStatistics()
    {
        XElement empty = XElement.Parse(
            $"""<office:meta xmlns:office="{OdfNamespaces.Office}" />""");

        DocumentMetadata metadata = OdfMetadata.Read(empty);
        metadata.Title.ShouldBeNull();
        metadata.Statistics.ShouldBeNull();
        metadata.Keywords.ShouldBeEmpty();
    }
}
