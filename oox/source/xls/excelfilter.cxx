/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excelfilter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:08 $
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

#include "oox/xls/excelfilter.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/xls/biffdetector.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/workbookfragment.hxx"
#include "oox/dump/biffdumper.hxx"
#include "oox/dump/xlsbdumper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::xml::sax::XFastDocumentHandler;
using ::oox::core::BinaryFilterBase;
using ::oox::core::FragmentHandlerRef;
using ::oox::core::RecordInfo;
using ::oox::core::RecordInfoProvider;
using ::oox::core::RecordInfoProviderRef;
using ::oox::core::Relation;
using ::oox::core::Relations;
using ::oox::core::XmlFilterBase;
using ::oox::vml::DrawingPtr;

namespace oox {
namespace xls {

// ============================================================================

namespace {

/** List of OOBIN record identifiers that start a new context level. */
static const struct RecordInfo spRecInfos[] =
{
    { OOBIN_ID_BOOKVIEWS,           OOBIN_ID_BOOKVIEWS + 1          },
    { OOBIN_ID_BORDERS,             OOBIN_ID_BORDERS + 1            },
    { OOBIN_ID_CELLSTYLES,          OOBIN_ID_CELLSTYLES + 1         },
    { OOBIN_ID_CELLSTYLEXFS,        OOBIN_ID_CELLSTYLEXFS + 1       },
    { OOBIN_ID_CELLXFS,             OOBIN_ID_CELLXFS + 1            },
    { OOBIN_ID_CFRULE,              OOBIN_ID_CFRULE + 1             },
    { OOBIN_ID_COLBREAKS,           OOBIN_ID_COLBREAKS + 1          },
    { OOBIN_ID_COLORS,              OOBIN_ID_COLORS + 1             },
    { OOBIN_ID_COLORSCALE,          OOBIN_ID_COLORSCALE + 1         },
    { OOBIN_ID_COLS,                OOBIN_ID_COLS + 1               },
    { OOBIN_ID_CONDFORMATTING,      OOBIN_ID_CONDFORMATTING + 1     },
    { OOBIN_ID_DATABAR,             OOBIN_ID_DATABAR + 1            },
    { OOBIN_ID_DATAVALIDATIONS,     OOBIN_ID_DATAVALIDATIONS + 1    },
    { OOBIN_ID_DDEITEMVALUES,       OOBIN_ID_DDEITEMVALUES + 1      },
    { OOBIN_ID_DXFS,                OOBIN_ID_DXFS + 1               },
    { OOBIN_ID_EXTERNALBOOK,        -1                              },
    { OOBIN_ID_EXTERNALREFS,        OOBIN_ID_EXTERNALREFS + 1       },
    { OOBIN_ID_EXTROW,              -1                              },
    { OOBIN_ID_EXTSHEETDATA,        OOBIN_ID_EXTSHEETDATA + 1       },
    { OOBIN_ID_FILLS,               OOBIN_ID_FILLS + 1              },
    { OOBIN_ID_FONTS,               OOBIN_ID_FONTS + 1              },
    { OOBIN_ID_HEADERFOOTER,        OOBIN_ID_HEADERFOOTER + 1       },
    { OOBIN_ID_ICONSET,             OOBIN_ID_ICONSET + 1            },
    { OOBIN_ID_INDEXEDCOLORS,       OOBIN_ID_INDEXEDCOLORS + 1      },
    { OOBIN_ID_MERGECELLS,          OOBIN_ID_MERGECELLS + 1         },
    { OOBIN_ID_MRUCOLORS,           OOBIN_ID_MRUCOLORS + 1          },
    { OOBIN_ID_NUMFMTS,             OOBIN_ID_NUMFMTS + 1            },
    { OOBIN_ID_ROW,                 -1                              },
    { OOBIN_ID_ROWBREAKS,           OOBIN_ID_ROWBREAKS + 1          },
    { OOBIN_ID_SHEETDATA,           OOBIN_ID_SHEETDATA + 1          },
    { OOBIN_ID_SHEETDATASET,        OOBIN_ID_SHEETDATASET + 1       },
    { OOBIN_ID_SHEETS,              OOBIN_ID_SHEETS + 1             },
    { OOBIN_ID_SHEETVIEW,           OOBIN_ID_SHEETVIEW + 1          },
    { OOBIN_ID_SHEETVIEWS,          OOBIN_ID_SHEETVIEWS + 1         },
    { OOBIN_ID_SST,                 OOBIN_ID_SST + 1                },
    { OOBIN_ID_STYLESHEET,          OOBIN_ID_STYLESHEET + 1         },
    { OOBIN_ID_TABLE,               OOBIN_ID_TABLE + 1              },
    { OOBIN_ID_TABLEPARTS,          OOBIN_ID_TABLEPARTS + 2         },  // end element increased by 2!
    { OOBIN_ID_TABLESTYLES,         OOBIN_ID_TABLESTYLES + 1        },
    { OOBIN_ID_VOLTYPE,             OOBIN_ID_VOLTYPE + 1            },
    { OOBIN_ID_VOLTYPEMAIN,         OOBIN_ID_VOLTYPEMAIN + 1        },
    { OOBIN_ID_VOLTYPES,            OOBIN_ID_VOLTYPES + 1           },
    { OOBIN_ID_WORKBOOK,            OOBIN_ID_WORKBOOK + 1           },
    { OOBIN_ID_WORKSHEET,           OOBIN_ID_WORKSHEET + 1          },
    { -1,                           -1                              }
};

} // namespace

// ============================================================================

OUString SAL_CALL ExcelFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelFilter" );
}

Sequence< OUString > SAL_CALL ExcelFilter_getSupportedServiceNames() throw()
{
    OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelFilter" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelFilter_createInstance(
        const Reference< XMultiServiceFactory >& rxFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelFilter( rxFactory ) );
}

// ----------------------------------------------------------------------------

ExcelFilter::ExcelFilter( const Reference< XMultiServiceFactory >& rxFactory ) :
    XmlFilterBase( rxFactory ),
    mpHelper( 0 )
{
}

ExcelFilter::~ExcelFilter()
{
}

bool ExcelFilter::importDocument() throw()
{
#if OOX_INCLUDE_DUMPER
    {
        ::oox::dump::xlsb::Dumper aDumper( *this );
        aDumper.dump();
        if( !aDumper.isImportEnabled() )
            return aDumper.isValid();
    }
#endif

    bool bRet = false;
    OUString aWorkbookPath = getFragmentPathFromType( CREATE_RELATIONS_TYPE( "officeDocument" ) );
    if( aWorkbookPath.getLength() > 0 )
    {
        WorkbookHelperRoot aHelper( *this );
        if( aHelper.isValid() )
        {
            mpHelper = &aHelper;    // needed for callbacks
            bRet = importFragment( new OoxWorkbookFragment( aHelper, aWorkbookPath ) );
            mpHelper = 0;
        }
    }
    return bRet;
}

bool ExcelFilter::exportDocument() throw()
{
    return false;
}

sal_Int32 ExcelFilter::getSchemeClr( sal_Int32 nColorSchemeToken ) const
{
    OSL_ENSURE( mpHelper, "ExcelFilter::getSchemeClr - no workbook helper" );
    return mpHelper ? mpHelper->getTheme().getColorByToken( nColorSchemeToken ) : -1;
}

const DrawingPtr ExcelFilter::getDrawings()
{
    return DrawingPtr();
}

RecordInfoProviderRef ExcelFilter::getRecordInfoProvider()
{
    if( !mxRecInfoProv )
        mxRecInfoProv.reset( new RecordInfoProvider( spRecInfos ) );
    return mxRecInfoProv;
}

OUString ExcelFilter::implGetImplementationName() const
{
    return ExcelFilter_getImplementationName();
}

// ============================================================================

OUString SAL_CALL ExcelBiffFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelBiffFilter" );
}

Sequence< OUString > SAL_CALL ExcelBiffFilter_getSupportedServiceNames() throw()
{
    OUString aServiceName = CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelBiffFilter" );
    Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

Reference< XInterface > SAL_CALL ExcelBiffFilter_createInstance(
        const Reference< XMultiServiceFactory >& rxFactory ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new ExcelBiffFilter( rxFactory ) );
}

// ----------------------------------------------------------------------------

ExcelBiffFilter::ExcelBiffFilter( const Reference< XMultiServiceFactory >& rxFactory ) :
    BinaryFilterBase( rxFactory )
{
}

ExcelBiffFilter::~ExcelBiffFilter()
{
}

bool ExcelBiffFilter::importDocument() throw()
{
#if OOX_INCLUDE_DUMPER
    {
        ::oox::dump::biff::Dumper aDumper( *this );
        aDumper.dump();
        if( !aDumper.isImportEnabled() )
            return aDumper.isValid();
    }
#endif

    bool bRet = false;

    // detect BIFF version and workbook stream name
    OUString aWorkbookName;
    BiffType eBiff = BiffDetector::detectStorageBiffVersion( aWorkbookName, getStorage() );
    BinaryInputStream aInStrm( getStorage()->openInputStream( aWorkbookName ), aWorkbookName.getLength() > 0 );
    OSL_ENSURE( (eBiff != BIFF_UNKNOWN) && aInStrm.is(), "ExcelBiffFilter::ExcelBiffFilter - invalid file format" );

    if( (eBiff != BIFF_UNKNOWN) && aInStrm.is() )
    {
        WorkbookHelperRoot aHelper( *this, eBiff );
        if( aHelper.isValid() )
        {
            BiffWorkbookFragment aFragment( aHelper );
            BiffInputStream aBiffStream( aInStrm );
            bRet = aFragment.importFragment( aBiffStream );
        }
    }
    return bRet;
}

bool ExcelBiffFilter::exportDocument() throw()
{
    return false;
}

OUString ExcelBiffFilter::implGetImplementationName() const
{
    return ExcelBiffFilter_getImplementationName();
}

// ============================================================================

} // namespace xls
} // namespace oox

