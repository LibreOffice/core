/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stylesfragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:06:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/stylesfragment.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxStylesFragment::OoxStylesFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath ),
    mfGradPos( -1.0 )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxStylesFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    sal_Int32 nCurrContext = getCurrentElement();
    switch( nCurrContext )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( styleSheet ));
        case XLS_TOKEN( styleSheet ):
            return  (nElement == XLS_TOKEN( colors )) ||
                    (nElement == XLS_TOKEN( fonts )) ||
                    (nElement == XLS_TOKEN( numFmts )) ||
                    (nElement == XLS_TOKEN( borders )) ||
                    (nElement == XLS_TOKEN( fills )) ||
                    (nElement == XLS_TOKEN( cellXfs )) ||
                    (nElement == XLS_TOKEN( cellStyleXfs )) ||
                    (nElement == XLS_TOKEN( dxfs )) ||
                    (nElement == XLS_TOKEN( cellStyles ));

        case XLS_TOKEN( colors ):
            return  (nElement == XLS_TOKEN( indexedColors ));
        case XLS_TOKEN( indexedColors ):
            return  (nElement == XLS_TOKEN( rgbColor ));

        case XLS_TOKEN( fonts ):
            return  (nElement == XLS_TOKEN( font ));
        case XLS_TOKEN( font ):
            return mxFont.get() && Font::isSupportedContext( nElement, nCurrContext );

        case XLS_TOKEN( numFmts ):
            return  (nElement == XLS_TOKEN( numFmt ));

        case XLS_TOKEN( borders ):
            return  (nElement == XLS_TOKEN( border ));
        case XLS_TOKEN( border ):
        case XLS_TOKEN( left ):
        case XLS_TOKEN( right ):
        case XLS_TOKEN( top ):
        case XLS_TOKEN( bottom ):
        case XLS_TOKEN( diagonal ):
            return mxBorder.get() && Border::isSupportedContext( nElement, nCurrContext );

        case XLS_TOKEN( fills ):
            return  (nElement == XLS_TOKEN( fill ));
        case XLS_TOKEN( fill ):
        case XLS_TOKEN( patternFill ):
        case XLS_TOKEN( gradientFill ):
        case XLS_TOKEN( stop ):
            return mxFill.get() && Fill::isSupportedContext( nElement, nCurrContext );

        case XLS_TOKEN( cellStyleXfs ):
        case XLS_TOKEN( cellXfs ):
            return  (nElement == XLS_TOKEN( xf ));
        case XLS_TOKEN( xf ):
            return mxXf.get() &&
                   ((nElement == XLS_TOKEN( alignment )) ||
                    (nElement == XLS_TOKEN( protection )));

        case XLS_TOKEN( dxfs ):
            return  (nElement == XLS_TOKEN( dxf ));
        case XLS_TOKEN( dxf ):
            return mxDxf.get() &&
                   ((nElement == XLS_TOKEN( font )) ||
                    (nElement == XLS_TOKEN( numFmt )) ||
                    (nElement == XLS_TOKEN( alignment )) ||
                    (nElement == XLS_TOKEN( protection )) ||
                    (nElement == XLS_TOKEN( border )) ||
                    (nElement == XLS_TOKEN( fill )));

        case XLS_TOKEN( cellStyles ):
            return  (nElement == XLS_TOKEN( cellStyle ));
    }
    return false;
}

void OoxStylesFragment::onStartElement( const AttributeList& rAttribs )
{
    sal_Int32 nCurrContext = getCurrentElement();
    sal_Int32 nPrevContext = getPreviousElement();

    switch( nCurrContext )
    {
        case XLS_TOKEN( color ):
            switch( nPrevContext )
            {
                case XLS_TOKEN( font ):
                    OSL_ENSURE( mxFont.get(), "OoxStylesFragment::onStartElement - missing font object" );
                    mxFont->importAttribs( nCurrContext, rAttribs );
                break;
                case XLS_TOKEN( stop ):
                    OSL_ENSURE( mxFill.get(), "OoxStylesFragment::onStartElement - missing fill object" );
                    mxFill->importColor( rAttribs, mfGradPos );
                break;
                default:
                    OSL_ENSURE( mxBorder.get(), "OoxStylesFragment::onStartElement - missing border object" );
                    mxBorder->importColor( nPrevContext, rAttribs );
            }
        break;
        case XLS_TOKEN( rgbColor ):
            getStyles().importPaletteColor( rAttribs );
        break;

        case XLS_TOKEN( font ):
            mxFont = mxDxf.get() ? mxDxf->importFont( rAttribs ) : getStyles().importFont( rAttribs );
        break;

        case XLS_TOKEN( numFmt ):
            if( mxDxf.get() )
                mxDxf->importNumFmt( rAttribs );
            else
                getStyles().importNumFmt( rAttribs );
        break;

        case XLS_TOKEN( alignment ):
            OSL_ENSURE( mxXf.get() || mxDxf.get(), "OoxStylesFragment::onStartElement - missing formatting object" );
            if( mxXf.get() )
                mxXf->importAlignment( rAttribs );
#if 0
            else if( mxDxf.get() )
                mxDxf->importAlignment( rAttribs );
#endif
        break;

        case XLS_TOKEN( protection ):
            OSL_ENSURE( mxXf.get() || mxDxf.get(), "OoxStylesFragment::onStartElement - missing formatting object" );
            if( mxXf.get() )
                mxXf->importProtection( rAttribs );
#if 0
            else if( mxDxf.get() )
                mxDxf->importProtection( rAttribs );
#endif
        break;

        case XLS_TOKEN( border ):
            mxBorder = mxDxf.get() ? mxDxf->importBorder( rAttribs ) : getStyles().importBorder( rAttribs );
        break;

        case XLS_TOKEN( fill ):
            mxFill = mxDxf.get() ? mxDxf->importFill( rAttribs ) : getStyles().importFill( rAttribs );
        break;
        case XLS_TOKEN( patternFill ):
            OSL_ENSURE( mxFill.get(), "OoxStylesFragment::onStartElement - missing fill object" );
            mxFill->importPatternFill( rAttribs );
        break;
        case XLS_TOKEN( fgColor ):
            OSL_ENSURE( mxFill.get(), "OoxStylesFragment::onStartElement - missing fill object" );
            mxFill->importFgColor( rAttribs );
        break;
        case XLS_TOKEN( bgColor ):
            OSL_ENSURE( mxFill.get(), "OoxStylesFragment::onStartElement - missing fill object" );
            mxFill->importBgColor( rAttribs );
        break;
        case XLS_TOKEN( gradientFill ):
            OSL_ENSURE( mxFill.get(), "OoxStylesFragment::onStartElement - missing fill object" );
            mxFill->importGradientFill( rAttribs );
        break;
        case XLS_TOKEN( stop ):
            mfGradPos = rAttribs.getDouble( XML_position, -1.0 );
        break;

        case XLS_TOKEN( xf ):
            mxXf = getStyles().importXf( nPrevContext, rAttribs );
        break;
        case XLS_TOKEN( dxf ):
            mxDxf = getStyles().importDxf( rAttribs );
        break;

        case XLS_TOKEN( cellStyle ):
            getStyles().importCellStyle( rAttribs );
        break;

        default: switch( nPrevContext )
        {
            case XLS_TOKEN( font ):
                OSL_ENSURE( mxFont.get(), "OoxStylesFragment::onStartElement - missing font object" );
                mxFont->importAttribs( nCurrContext, rAttribs );
            break;
            case XLS_TOKEN( border ):
                OSL_ENSURE( mxBorder.get(), "OoxStylesFragment::onStartElement - missing border object" );
                mxBorder->importStyle( nCurrContext, rAttribs );
            break;
        }
    }
}

void OoxStylesFragment::onEndElement( const OUString& /*rChars*/ )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( font ):     mxFont.reset();     break;
        case XLS_TOKEN( border ):   mxBorder.reset();   break;
        case XLS_TOKEN( fill ):     mxFill.reset();     break;
        case XLS_TOKEN( xf ):       mxXf.reset();       break;
        case XLS_TOKEN( dxf ):      mxDxf.reset();      break;
    }
}

ContextWrapper OoxStylesFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_STYLESHEET);
        case OOBIN_ID_STYLESHEET:
            return  (nRecId == OOBIN_ID_COLORS) ||
                    (nRecId == OOBIN_ID_FONTS) ||
                    (nRecId == OOBIN_ID_NUMFMTS) ||
                    (nRecId == OOBIN_ID_BORDERS) ||
                    (nRecId == OOBIN_ID_FILLS) ||
                    (nRecId == OOBIN_ID_CELLSTYLEXFS) ||
                    (nRecId == OOBIN_ID_CELLXFS) ||
                    (nRecId == OOBIN_ID_DXFS) ||
                    (nRecId == OOBIN_ID_CELLSTYLES);
        case OOBIN_ID_COLORS:
            return  (nRecId == OOBIN_ID_INDEXEDCOLORS);
        case OOBIN_ID_INDEXEDCOLORS:
            return  (nRecId == OOBIN_ID_RGBCOLOR);
        case OOBIN_ID_FONTS:
            return  (nRecId == OOBIN_ID_FONT);
        case OOBIN_ID_NUMFMTS:
            return  (nRecId == OOBIN_ID_NUMFMT);
        case OOBIN_ID_BORDERS:
            return  (nRecId == OOBIN_ID_BORDER);
        case OOBIN_ID_FILLS:
            return  (nRecId == OOBIN_ID_FILL);
        case OOBIN_ID_CELLSTYLEXFS:
        case OOBIN_ID_CELLXFS:
            return  (nRecId == OOBIN_ID_XF);
        case OOBIN_ID_DXFS:
            return  (nRecId == OOBIN_ID_DXF);
        case OOBIN_ID_CELLSTYLES:
            return  (nRecId == OOBIN_ID_CELLSTYLE);
    }
    return false;
}

void OoxStylesFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_RGBCOLOR:     getStyles().importPaletteColor( rStrm );                break;
        case OOBIN_ID_FONT:         getStyles().importFont( rStrm );                        break;
        case OOBIN_ID_NUMFMT:       getStyles().importNumFmt( rStrm );                      break;
        case OOBIN_ID_BORDER:       getStyles().importBorder( rStrm );                      break;
        case OOBIN_ID_FILL:         getStyles().importFill( rStrm );                        break;
        case OOBIN_ID_XF:           getStyles().importXf( getPreviousElement(), rStrm );    break;
        case OOBIN_ID_DXF:          getStyles().importDxf( rStrm );                         break;
        case OOBIN_ID_CELLSTYLE:    getStyles().importCellStyle( rStrm );                   break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxStylesFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_BORDERS,         OOBIN_ID_BORDERS + 1        },
        { OOBIN_ID_CELLSTYLES,      OOBIN_ID_CELLSTYLES + 1     },
        { OOBIN_ID_CELLSTYLEXFS,    OOBIN_ID_CELLSTYLEXFS + 1   },
        { OOBIN_ID_CELLXFS,         OOBIN_ID_CELLXFS + 1        },
        { OOBIN_ID_COLORS,          OOBIN_ID_COLORS + 1         },
        { OOBIN_ID_DXFS,            OOBIN_ID_DXFS + 1           },
        { OOBIN_ID_FILLS,           OOBIN_ID_FILLS + 1          },
        { OOBIN_ID_FONTS,           OOBIN_ID_FONTS + 1          },
        { OOBIN_ID_INDEXEDCOLORS,   OOBIN_ID_INDEXEDCOLORS + 1  },
        { OOBIN_ID_MRUCOLORS,       OOBIN_ID_MRUCOLORS + 1      },
        { OOBIN_ID_NUMFMTS,         OOBIN_ID_NUMFMTS + 1        },
        { OOBIN_ID_STYLESHEET,      OOBIN_ID_STYLESHEET + 1     },
        { OOBIN_ID_TABLESTYLES,     OOBIN_ID_TABLESTYLES + 1    },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

void OoxStylesFragment::finalizeImport()
{
    getStyles().finalizeImport();
}

// ============================================================================

} // namespace xls
} // namespace oox

