# Office Open XML (ooxml) Support

Support for Office Open XML, the office XML-format designed by Microsoft.

## DrawingML Custom shapes and presets

Custom shapes are part of DrawingML and are different to binary ppt
and VML in older formats.
The import happens in `oox/source/drawingml`, where they are
imported as LO's enhanced custom shape's. see
`offapi/com/sun/star/drawing/CustomShape.idl` and
`offapi/com/sun/star/drawing/EnhancedCustomShape*.idl`
Check `CustomShapeProperties::pushToPropSet()` and see
how custom shape properties are converted.

Preset shapes are custom shapes whose guides and handles
have been defined in OOXML specification. By specifying
preset type and the adjustment values, the reset can
be taken from the shape definition.

example of drawingml preset:

         <a:prstGeom prst="star5">
           <a:avLst/>
         </a:prstGeom>

example of drawingml custom shape (equal to star5 preset):

   <avLst xmlns="http://schemas.openxmlformats.org/drawingml/2006/main">
     <gd name="adj" fmla="val 19098" />
     <gd name="hf" fmla="val 105146" />
     <gd name="vf" fmla="val 110557" />
   </avLst>
   <gdLst xmlns="http://schemas.openxmlformats.org/drawingml/2006/main">
     <gd name="a" fmla="pin 0 adj 50000" />
     <gd name="swd2" fmla="*/ wd2 hf 100000" />
     <gd name="shd2" fmla="*/ hd2 vf 100000" />
     <gd name="svc" fmla="*/ vc  vf 100000" />
     <gd name="dx1" fmla="cos swd2 1080000" />
     <gd name="dx2" fmla="cos swd2 18360000" />
     <gd name="dy1" fmla="sin shd2 1080000" />
     <gd name="dy2" fmla="sin shd2 18360000" />
     <gd name="x1" fmla="+- hc 0 dx1" />
     <gd name="x2" fmla="+- hc 0 dx2" />
     <gd name="x3" fmla="+- hc dx2 0" />
     <gd name="x4" fmla="+- hc dx1 0" />
     <gd name="y1" fmla="+- svc 0 dy1" />
     <gd name="y2" fmla="+- svc 0 dy2" />
     <gd name="iwd2" fmla="*/ swd2 a 50000" />
     <gd name="ihd2" fmla="*/ shd2 a 50000" />
     <gd name="sdx1" fmla="cos iwd2 20520000" />
     <gd name="sdx2" fmla="cos iwd2 3240000" />
     <gd name="sdy1" fmla="sin ihd2 3240000" />
     <gd name="sdy2" fmla="sin ihd2 20520000" />
     <gd name="sx1" fmla="+- hc 0 sdx1" />
     <gd name="sx2" fmla="+- hc 0 sdx2" />
     <gd name="sx3" fmla="+- hc sdx2 0" />
     <gd name="sx4" fmla="+- hc sdx1 0" />
     <gd name="sy1" fmla="+- svc 0 sdy1" />
     <gd name="sy2" fmla="+- svc 0 sdy2" />
     <gd name="sy3" fmla="+- svc ihd2 0" />
     <gd name="yAdj" fmla="+- svc 0 ihd2" />
   </gdLst>
   <ahLst xmlns="http://schemas.openxmlformats.org/drawingml/2006/main">
     <ahXY gdRefY="adj" minY="0" maxY="50000">
       <pos x="hc" y="yAdj" />
     </ahXY>
   </ahLst>
   <cxnLst xmlns="http://schemas.openxmlformats.org/drawingml/2006/main">
     <cxn ang="3cd4">
       <pos x="hc" y="t" />
     </cxn>
     <cxn ang="cd2">
       <pos x="x1" y="y1" />
     </cxn>
     <cxn ang="cd4">
       <pos x="x2" y="y2" />
     </cxn>
     <cxn ang="cd4">
       <pos x="x3" y="y2" />
     </cxn>
     <cxn ang="0">
       <pos x="x4" y="y1" />
     </cxn>
   </cxnLst>
   <rect l="sx1" t="sy1" r="sx4" b="sy3" xmlns="http://schemas.openxmlformats.org/drawingml/2006/main" />
   <pathLst xmlns="http://schemas.openxmlformats.org/drawingml/2006/main">
     <path>
       <moveTo>
         <pt x="x1" y="y1" />
       </moveTo>
       <lnTo>
         <pt x="sx2" y="sy1" />
       </lnTo>
       <lnTo>
         <pt x="hc" y="t" />
       </lnTo>
       <lnTo>
         <pt x="sx3" y="sy1" />
       </lnTo>
       <lnTo>
         <pt x="x4" y="y1" />
       </lnTo>
       <lnTo>
         <pt x="sx4" y="sy2" />
       </lnTo>
       <lnTo>
         <pt x="x3" y="y2" />
       </lnTo>
       <lnTo>
         <pt x="hc" y="sy3" />
       </lnTo>
       <lnTo>
         <pt x="x2" y="y2" />
       </lnTo>
       <lnTo>
         <pt x="sx1" y="sy2" />
       </lnTo>
       <close />
     </path>
   </pathLst>

we needed to extend our custom shapes for missing features and so 5
new segment commands were added. `G` command for arcto drawingml record
and `H` `I` `J` `K` commands for darken, darkenless, lighten, lightenless
records. the commands are save into ODF in special namespace drawooo,
which is extension not yet in the standard. Thorsten suggested to put
it in such a namespace and keep original (incomplete) geometry for
backward compatibility, before we can extend the ODF. that's why you
will see 2 of them in cases where some of the new commands was
needed.

In order to convert preset shapes to LO's enhanced custom shape,
we need to load shape definition of preset shapes. The procedure
to convert the definition from OOXML spec for LO is documented
(also a script) in `oox/source/drawingml/customshapes/README.md`.
The scripts in `oox/source/drawingml/customshapes/` also generate pptx
files for single presets and also for all presets
`cshape-all.pptx`. The cshape-all.pptx file is then loaded into Impress
build with debug enabled in oox and the command line output contains
information. The generated definition is `oox-drawingml-cs-presets`.

Check `CustomShapeProperties::initializePresetDataMap()` to see how
generated presets data are loaded into LO.
While importing presets, we prefix the name with "ooxml-" so
that we can detect it on export as save it again as preset.

The generated pptx files
can be used when debugging bugs in custom shapes import/export. also
the cshape-all.pptx can be used to test the round trips. there's small
problem with these pptx as they cannot be imported into powerpoint,
but that can be fixed quickly. when fixed, we can use it to
test powerpoint odp export and see how complete it is regarding
custom shapes. OpenXML SDK tools might help when fixing
`cshape-all.pptx`
<http://www.microsoft.com/en-us/download/details.aspx?id=30425>

## Export
Here is how LO's enhanced custom shapes are exported:
* Shape name is `ooxml-*` - they are imported from ooxml, export as is.
* Denylist - ODF presets that has OOXML equivalent.
  We convert adjustment values case by case. Microsoft Office
  is rather strict about adjustment values, either none of them
  are provided so that default values are taken, or the exact set
  of handles have to be provided. In some cases we are converting
  from the preset with less handles to the one with more handles
  so that default values suitable for the odf one need to be
  provided.
* Allowlist - ODF presets that has OOXML equivalent but looks a bit
different, export them as `PolyPolygon`.

Check Andras Timar's presentation[1] and `ShapeExport::WriteCustomShape()`
for further detail.

## Future Works
Because we have to make sure that all the roundtrips
like PPTX --> ODP --> PPTX work correctly and doesn't lose data.
the only problematic part is probably saving custom shapes (ie. not
presets) to PPTX. that part of code predates work on custom shapes
and is unable to export general custom shapes yet. It will need a bit
of work as LO has more complex equations than `DrawingML`. other parts
should work OK, PPTX --> ODP should work and don't lose any
data. presets should already survive PPTX --> ODP --> PPTX roundtrip

[1] <https://archive.fosdem.org/2016/schedule/event/drawingml/attachments/slides/1184/export/events/attachments/drawingml/slides/1184/andras_timar_fosdem_2016.pdf>
