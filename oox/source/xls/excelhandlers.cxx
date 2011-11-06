/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/xls/excelhandlers.hxx"

#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using ::oox::core::FilterBase;
using ::oox::core::FragmentHandler2;
using ::rtl::OUString;

// ============================================================================
// ============================================================================

WorkbookFragmentBase::WorkbookFragmentBase(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorkbookHelper( rHelper )
{
}

// ============================================================================

WorksheetFragmentBase::WorksheetFragmentBase(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    FragmentHandler2( rHelper.getOoxFilter(), rFragmentPath ),
    WorksheetHelper( rHelper )
{
}

// ============================================================================
// ============================================================================

BiffContextHandler::~BiffContextHandler()
{
}

// ----------------------------------------------------------------------------

BiffWorkbookContextBase::BiffWorkbookContextBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

// ----------------------------------------------------------------------------

BiffWorksheetContextBase::BiffWorksheetContextBase( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

// ============================================================================

namespace {

const sal_uInt16 BIFF_BOF_GLOBALS           = 0x0005;   /// BIFF5-BIFF8 workbook globals.
const sal_uInt16 BIFF_BOF_MODULE            = 0x0006;   /// BIFF5-BIFF8 Visual Basic module.
const sal_uInt16 BIFF_BOF_SHEET             = 0x0010;   /// BIFF2-BIFF8 worksheet/dialog sheet.
const sal_uInt16 BIFF_BOF_CHART             = 0x0020;   /// BIFF2-BIFF8 chart sheet.
const sal_uInt16 BIFF_BOF_MACRO             = 0x0040;   /// BIFF4-BIFF8 macro sheet.
const sal_uInt16 BIFF_BOF_WORKSPACE         = 0x0100;   /// BIFF3-BIFF8 workspace.

} // namespace

// ----------------------------------------------------------------------------

BiffFragmentHandler::BiffFragmentHandler( const FilterBase& rFilter, const OUString& rStrmName )
{
    // do not automatically close the root stream (indicated by empty stream name)
    bool bRootStrm = rStrmName.getLength() == 0;
    mxXInStrm.reset( new BinaryXInputStream( rFilter.openInputStream( rStrmName ), !bRootStrm ) );
    mxBiffStrm.reset( new BiffInputStream( *mxXInStrm ) );
}

BiffFragmentHandler::~BiffFragmentHandler()
{
}

BiffFragmentType BiffFragmentHandler::startFragment( BiffType eBiff )
{
    BiffFragmentType eFragment = BIFF_FRAGMENT_UNKNOWN;
    /*  #i23425# Don't rely on BOF record ID to read BOF contents, but on
        the detected BIFF version. */
    if( mxBiffStrm->startNextRecord() && BiffHelper::isBofRecord( *mxBiffStrm ) )
    {
        // BOF is always written unencrypted
        mxBiffStrm->enableDecoder( false );
        mxBiffStrm->skip( 2 );
        sal_uInt16 nType = mxBiffStrm->readuInt16();

        // decide which fragment types are valid for current BIFF version
        switch( eBiff )
        {
            case BIFF2: switch( nType )
            {
                case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                // #i51490# Excel interprets invalid types as worksheet
                default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
            }
            break;

            case BIFF3: switch( nType )
            {
                case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_UNKNOWN;      break;
                // #i51490# Excel interprets invalid types as worksheet
                default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
            };
            break;

            case BIFF4: switch( nType )
            {
                case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_EMPTYSHEET;   break;
                case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_WORKSPACE;    break;
                // #i51490# Excel interprets invalid types as worksheet
                default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
            };
            break;

            case BIFF5:
            case BIFF8: switch( nType )
            {
                case BIFF_BOF_GLOBALS:  eFragment = BIFF_FRAGMENT_GLOBALS;      break;
                case BIFF_BOF_CHART:    eFragment = BIFF_FRAGMENT_CHARTSHEET;   break;
                case BIFF_BOF_MACRO:    eFragment = BIFF_FRAGMENT_MACROSHEET;   break;
                case BIFF_BOF_MODULE:   eFragment = BIFF_FRAGMENT_MODULESHEET;  break;
                case BIFF_BOF_WORKSPACE:eFragment = BIFF_FRAGMENT_UNKNOWN;      break;
                // #i51490# Excel interprets invalid types as worksheet
                default:                eFragment = BIFF_FRAGMENT_WORKSHEET;
            };
            break;

            case BIFF_UNKNOWN: break;
        }
    }
    return eFragment;
}

bool BiffFragmentHandler::skipFragment()
{
    while( mxBiffStrm->startNextRecord() && (mxBiffStrm->getRecId() != BIFF_ID_EOF) )
        if( BiffHelper::isBofRecord( *mxBiffStrm ) )
            skipFragment();
    return !mxBiffStrm->isEof() && (mxBiffStrm->getRecId() == BIFF_ID_EOF);
}

// ============================================================================

BiffWorkbookFragmentBase::BiffWorkbookFragmentBase( const WorkbookHelper& rHelper, const OUString& rStrmName, bool bCloneDecoder ) :
    BiffFragmentHandler( rHelper.getBaseFilter(), rStrmName ),
    WorkbookHelper( rHelper )
{
    if( bCloneDecoder )
        getCodecHelper().cloneDecoder( getInputStream() );
}

// ----------------------------------------------------------------------------

BiffWorksheetFragmentBase::BiffWorksheetFragmentBase( const WorksheetHelper& rHelper, const BiffWorkbookFragmentBase& rParent ) :
    BiffFragmentHandler( rParent ),
    WorksheetHelper( rHelper )
{
}

// ----------------------------------------------------------------------------

BiffSkipWorksheetFragment::BiffSkipWorksheetFragment( const WorksheetHelper& rHelper, const BiffWorkbookFragmentBase& rParent ) :
    BiffWorksheetFragmentBase( rHelper, rParent )
{
}

bool BiffSkipWorksheetFragment::importFragment()
{
    return skipFragment();
}

// ============================================================================
// ============================================================================

} // namespace xls
} // namespace oox
