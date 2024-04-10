#!/usr/bin/python3

import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
readSet = set()
writeSet = set()
sourceLocationSet = set()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

def parseFieldInfo( tokens ):
    if len(tokens) == 3:
        return (normalizeTypeParams(tokens[1]), tokens[2])
    else:
        return (normalizeTypeParams(tokens[1]), "")

with io.open("workdir/loplugin.unusedenumconstants.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            srcLoc = tokens[3]
            # ignore external source code
            if (srcLoc.startswith("external/")):
                continue
            # ignore build folder
            if (srcLoc.startswith("workdir/")):
                continue
            definitionSet.add(fieldInfo)
            definitionToSourceLocationMap[fieldInfo] = srcLoc
        elif tokens[0] == "read:":
            readSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "write:":
            writeSet.add(parseFieldInfo(tokens))
        else:
            print( "unknown line: " + line)

def startswith_one_of( srcLoc, fileSet ):
    for f in fileSet:
        if srcLoc.startswith(f):
            return True;
    return False;

def is_ignore(srcLoc):
    if startswith_one_of(srcLoc,
        [
        # this is all representations of on-disk or external data structures
         "basic/source/inc/filefmt.hxx",
         "basic/source/sbx/sbxscan.cxx",
         "cppcanvas/source/mtfrenderer/emfpbrush.hxx",
         "filter/source/graphicfilter/ipcd/ipcd.cxx",
         "filter/source/t602/t602filter.hxx",
         "include/filter/msfilter/escherex.hxx",
         "include/filter/msfilter/svdfppt.hxx",
         "hwpfilter/",
         "include/registry/types.hxx",
         "lotuswordpro/",
         "include/sot/formats.hxx",
         "include/svx/msdffdef.hxx",
         "sc/source/filter/inc/xlconst.hxx",
         "include/unotools/saveopt.hxx",
         "sw/inc/fldbas.hxx",
         "sw/source/filter/inc/wwstyles.hxx",
         "sw/source/filter/ww8/fields.hxx",
         "vcl/source/fontsubset/cff.cxx",
         "include/vcl/settings.hxx", # stored in a setting, can't remove it without potentially triggering UBSAN
         "basic/source/inc/opcodes.hxx", # can't touch this without breaking unit tests, not sure why
         "include/unotools/securityoptions.hxx", # comes from the UI
         "sot/source/sdstor/stgelem.hxx",
         "sd/source/filter/eppt/epptbase.hxx",
         "include/registry/refltype.hxx",
         "include/registry/version.h",
         "include/svtools/rtftoken.h",
         "sc/source/filter/inc/xltracer.hxx",
         "sw/source/writerfilter/dmapper/FieldTypes.hxx",
         "vcl/source/fontsubset/cff.cxx",
         "vcl/source/filter/wmf/winmtf.hxx",
         "vcl/source/filter/sgvmain.hxx",
         "vcl/source/filter/jpeg/transupp.h",
         "include/vcl/bitmapex.hxx", # TransparentType
         "vcl/inc/sft.hxx", # CompositeFlags, WidthClass, WeightClass
         "vcl/inc/CommonSalLayout.hxx", # VerticalOrientation
         "include/tools/fontenum.hxx", # part of GDI file format
         "svx/inc/galobj.hxx", # GalSoundType
         "include/svx/msdffdef.hxx",
         "include/svtools/rtftoken.h", # RTF_TOKEN_IDS
         "starmath/source/mathtype.hxx", # MathType::MTOKENS
         "sd/source/filter/eppt/epptbase.hxx", # PPTExTextAttr
         "sc/source/filter/inc/tokstack.hxx", # E_TYPE
         "filter/source/graphicfilter/icgm/cgmtypes.hxx",
         "basic/source/inc/filefmt.hxx", # FileOffset
         "include/basic/sbxdef.hxx", # SbxDataType
         "connectivity/source/inc/dbase/DTable.hxx", # ODbaseTable::DBFType
         "codemaker/source/javamaker/classfile.hxx", # AccessFlags
         "basic/source/inc/filefmt.hxx", # FileOffset
         "basic/source/inc/opcodes.hxx", #  SbiOpcode
         "sc/source/filter/inc/flttypes.hxx", # BiffTyp
         "sc/inc/optutil.hxx", # ScOptionsUtil::KeyBindingType
         "include/sfx2/chalign.hxx", # SfxChildAlignment
         "drawinglayer/source/tools/emfpbrush.hxx",
         "drawinglayer/source/tools/emfppen.cxx",
         "include/oox/ppt/animationspersist.hxx",
         "include/vcl/fontcapabilities.hxx",
         "sw/inc/poolfmt.hxx",
         "include/xmloff/xmltoken.hxx",
        # unit test code
         "cppu/source/uno/check.cxx",
        # general weird nonsense going on
         "framework/inc/helper/mischelper.hxx"
         "include/sfx2/shell.hxx",
         "framework/inc/helper/mischelper.hxx",
         "include/svtools/htmltokn.h",
         "include/sfx2/shell.hxx",
         "sw/inc/iodetect.hxx",
         "sw/inc/fmtfordr.hxx",
         "sw/inc/flddat.hxx",
         "sw/source/uibase/config/modcfg.cxx", # InsertConfigProp
         "sw/inc/calc.hxx", # SwCalcOper
         "svtools/source/config/helpopt.cxx", # HelpProperty
         "include/svtools/htmltokn.h",
         "include/sfx2/sidebar/Theme.hxx", # ThemeItem
         "sc/source/ui/docshell/impex.cxx", # SylkVersion
         "include/i18nutil/paper.hxx", # Paper
         "cppcanvas/source/mtfrenderer/emfplus.cxx", # EmfPlusCombineMode
         "cppcanvas/source/mtfrenderer/emfpbrush.hxx", # EmfPlusHatchStyle
         "include/filter/msfilter/svdfppt.hxx", # PptPlaceholder, PptSlideLayout
         "include/filter/msfilter/escherex.hxx", # various
         "basic/source/inc/opcodes.hxx", # SbiOpcode
         "basic/source/inc/token.hxx", # SbiToken
         "binaryurp/source/specialfunctionids.hxx", # binaryurp::SpecialFunctionIds
         "connectivity/source/inc/odbc/OTools.hxx", # ODBC3SQLFunctionId
         "include/formula/grammar.hxx", # FormulaGrammar::Grammar
         "basic/source/sbx/sbxres.hxx", # StringId
         "sc/source/core/tool/chartpos.cxx", # CellState
         "sc/source/core/data/global2.cxx", # State
         "sc/inc/global.hxx", # ScAggregateFunc
         "registry/source/reflcnst.hxx", #CPInfoTag
        # Windows or OSX only
         "include/canvas/rendering/icolorbuffer.hxx",
         "include/vcl/commandevent.hxx",
         "vcl/inc/unx/gendata.hxx",
         "vcl/inc/salwtype.hxx",
         "include/vcl/svapp.hxx",
         "include/vcl/commandevent.hxx", # CommandEvent, MediaCommand, ShowDialogId
         "include/canvas/rendering/irendermodule.hxx", # canvas::IRenderModule::PrimitiveType
         "sal/osl/unx/file.cxx", # FileHandle_Impl::Kind
        # must match some other enum
         "include/editeng/bulletitem.hxx",
         "include/editeng/svxenum.hxx",
         "include/formula/opcode.hxx", # OpCode
         "include/i18nutil/paper.hxx",
         "include/oox/drawingml/shapepropertymap.hxx",
         "include/svl/nfkeytab.hx",
         "include/svl/zforlist.hxx",
         "include/vcl/svtabbx.hxx",
         "include/vcl/print.hxx", # NupOrderType, from UI combobox
         "sw/source/uibase/inc/swcont.hxx", # RegionMode, from UI; ContentTypeId, from UI(registry)
         "sw/inc/toxe.hxx", # ToxAuthorityType (from UI)
         "include/svx/sxekitm.hxx", # SdrEdgeKind (from UI)
         "include/svx/paraprev.hxx", # SvxPrevLineSpace (from UI)
         "include/svx/ctredlin.hxx", # SvxRedlinDateMode (from UI)
         "sd/source/ui/inc/animobjs.hxx", # BitmapAdjustment (from UI)
         "sd/source/ui/dlg/PhotoAlbumDialog.hxx", # SlideImageLayout (from UI)
         "sd/inc/pres.hxx", # AutoLayout (from UI)
         "sc/source/ui/inc/scuitphfedit.hxx", # ScHFEntryId (from UI)
         "include/i18nlangtag/languagetag.hxx", # LanguageTag::ScriptType
         "extensions/source/scanner/grid.hxx", # ResetType (from UI)
         "dbaccess/source/inc/dsntypes.hxx", # dbaccess::DATASOURCE_TYPE (from UI)
         "cui/source/tabpages/tparea.cxx", # FillType (from UI)
         "include/editeng/svxenum.hxx", # css::style::NumberingType
         "include/editeng/bulletitem.hxx", # css::style::NumberingType
         "basic/source/sbx/sbxdec.hxx", # SbxDecimal::CmpResult, must match some Windows API
         "codemaker/source/javamaker/javatype.cxx", # TypeInfo::Flags, from UNO
         "chart2/source/view/inc/AbstractShapeFactory.hxx", # chart::SymbolEnum, called via int UNO param
        # represents constants from an external API
         "opencl/inc/opencl_device_selection.h",
         "vcl/inc/sft.hxx",
         "vcl/unx/gtk/xid_fullscreen_on_all_monitors.c",
         "vcl/unx/gtk/salnativewidgets-gtk.cxx",
         "sc/inc/callform.hxx", # ParamType
         "include/i18nlangtag/applelangid.hxx", # AppleLanguageId
         "connectivity/source/drivers/firebird/Util.hxx", # firebird::BlobSubtype
         "include/xmloff/xmltoken.hxx",
         "sw/source/writerfilter/rtftok/rtfcontrolwords.hxx",
         ]):
         return True
    if d[1] == "UNKNOWN" or d[1] == "LAST" or d[1].endswith("NONE") or d[1].endswith("None") or d[1].endswith("EQUAL_SIZE"):
        return True
    return False


untouchedSet = set()
for d in definitionSet:
    if d in readSet or d in writeSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    if (is_ignore(srcLoc)):
        continue

    untouchedSet.add((d[0] + " " + d[1], srcLoc))

writeonlySet = set()
for d in writeSet:
    if d in readSet:
        continue
    # can happen with stuff in workdir or external
    if d not in definitionSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    if (is_ignore(srcLoc)):
        continue
    writeonlySet.add((d[0] + " " + d[1], srcLoc))

readonlySet = set()
for d in readSet:
    if d in writeSet:
        continue
    # can happen with stuff in workdir or external
    if d not in definitionSet:
        continue
    srcLoc = definitionToSourceLocationMap[d];
    if (is_ignore(srcLoc)):
        continue
    readonlySet.add((d[0] + " " + d[1], srcLoc))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp1list = sorted(untouchedSet, key=lambda v: v_sort_key(v))
tmp2list = sorted(writeonlySet, key=lambda v: v_sort_key(v))
tmp3list = sorted(readonlySet, key=lambda v: v_sort_key(v))

# print out the results
with open("compilerplugins/clang/unusedenumconstants.untouched.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedenumconstants.writeonly.results", "wt") as f:
    for t in tmp2list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
with open("compilerplugins/clang/unusedenumconstants.readonly.results", "wt") as f:
    for t in tmp3list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )


