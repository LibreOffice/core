# Paperless.OpenDocument — TODO

Shared ODF infrastructure: style families and resolution, common attributes, flat XML.

**Implement this before OOXML.** ODF is XML, well-specified, and exercises the same
style-resolution machinery OOXML needs — so it is both the easier target and the one that
de-risks the harder one.

Reference: `research/02-writer.md` section D; `research/05-infrastructure.md` section E for
the item-set semantics being reproduced.

## Style resolution

The core semantic, and the thing to get exactly right.

- [ ] Parse the three style containers, all of which participate:
      `office:styles` (named), `office:automatic-styles` (generated, standing in for direct
      formatting), `office:master-styles` (page and slide masters)
- [ ] Walk `style:parent-style-name` upwards, then fall back to `style:default-style` for
      the family
- [ ] **Keep "set here", "inherited" and "defaulted" distinguishable.** Collapsing them into
      "has a value" loses information later code needs — this is the `SfxItemSet`
      `SfxItemState` distinction LibreOffice maintains for the same reason.
- [ ] All style families (`OdfStyleFamily`)
- [ ] `style:*-properties` child elements per family
- [ ] Data styles (`number:`) for number formatting

## Common parsing

- [ ] ODF length values with units (`cm`, `mm`, `in`, `pt`, `pc`, `px`) → `Length`
- [ ] Percentages, and where they resolve against
- [ ] Colours (`#rrggbb`, `transparent`)
- [ ] `fo:` formatting attributes: borders, padding, margins, backgrounds
- [ ] `svg:` position and size attributes
- [ ] Boolean and enumerated attribute values

## Flat XML

- [ ] `.fodt`/`.fods`/`.fodp`: one document with `office:body` inline, no package.
      Present it through the same interface as a packaged document so nothing above cares.

## Metadata

- [ ] `meta.xml`: Dublin Core plus `meta:*`, including `meta:document-statistic`
- [ ] `settings.xml` where it affects rendering (view settings mostly do not; some
      compatibility flags do)

## Legacy OpenOffice.org 1.x

- [ ] `sxw`/`sxc`/`sxi`: same shape, older namespaces. Low priority — rare in practice.
