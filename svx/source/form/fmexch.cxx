/*************************************************************************
 *
 *  $RCSfile: fmexch.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2003-08-07 11:50:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_FMEXCH_HXX
#include "fmexch.hxx"
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_DBEXCH_HRC
#include <dbexch.hrc>
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

static sal_uInt32 nFieldFormat = 0;
static sal_uInt32 nControlFormat = 0;

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::datatransfer;

    //====================================================================
    //= OLocalExchange
    //====================================================================
    //--------------------------------------------------------------------
    OLocalExchange::OLocalExchange( )
        :m_bDragging( sal_False )
        ,m_bClipboardOwner( sal_False )
    {
    }

    //--------------------------------------------------------------------
    void OLocalExchange::copyToClipboard( Window* _pWindow, const GrantAccess& )
    {
        if ( m_bClipboardOwner )
        {   // simulate a lostOwnership to notify parties interested in
            if ( m_aClipboardListener.IsSet() )
                m_aClipboardListener.Call( this );
        }

        m_bClipboardOwner = sal_True;
        CopyToClipboard( _pWindow );
    }

    //--------------------------------------------------------------------
    void OLocalExchange::clear()
    {
        if ( isClipboardOwner() )
        {
            try
            {
                Reference< clipboard::XClipboard > xClipBoard( getOwnClipboard() );
                if ( xClipBoard.is() )
                    xClipBoard->setContents( NULL, NULL );
            }
            catch( const Exception& e )
            {
                e;  // make compiler happy
                DBG_ERROR( "OLocalExchange::clear: could not reset the clipboard!" );
            }
            m_bClipboardOwner = sal_False;
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OLocalExchange::lostOwnership( const Reference< clipboard::XClipboard >& _rxClipboard, const Reference< XTransferable >& _rxTrans ) throw(RuntimeException)
    {
        TransferableHelper::implCallOwnLostOwnership( _rxClipboard, _rxTrans );
        m_bClipboardOwner = sal_False;

        if ( m_aClipboardListener.IsSet() )
            m_aClipboardListener.Call( this );
    }

    //--------------------------------------------------------------------
    void OLocalExchange::startDrag( Window* _pWindow, sal_Int8 _nDragSourceActions, const GrantAccess& )
    {
        m_bDragging = sal_True;
        StartDrag( _pWindow, _nDragSourceActions );
    }

    //--------------------------------------------------------------------
    void OLocalExchange::DragFinished( sal_Int8 nDropAction )
    {
        TransferableHelper::DragFinished( nDropAction );
        m_bDragging = sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OLocalExchange::hasFormat( const DataFlavorExVector& _rFormats, sal_uInt32 _nFormatId )
    {
        for (   DataFlavorExVector::const_iterator aSearch = _rFormats.begin();
                aSearch != _rFormats.end();
                ++aSearch
            )
            if ( aSearch->mnSotId == _nFormatId )
                break;

        return aSearch != _rFormats.end();
    }

    //--------------------------------------------------------------------
    sal_Bool OLocalExchange::GetData( const ::com::sun::star::datatransfer::DataFlavor& _rFlavor )
    {
        return sal_False;   // do not have any formats by default
    }

    //====================================================================
    //= OControlTransferData
    //====================================================================
    //--------------------------------------------------------------------
    OControlTransferData::OControlTransferData( )
        :m_pFocusEntry( NULL )
    {
    }

    //--------------------------------------------------------------------
    OControlTransferData::OControlTransferData( const Reference< XTransferable >& _rxTransferable )
        :m_pFocusEntry( NULL )
    {
        TransferableDataHelper aExchangedData( _rxTransferable );

        // try the formats we know
        if ( OControlExchange::hasControlPathFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // paths to the controls, relative to a root
            Sequence< Any > aControlPathData;
            if ( aExchangedData.GetAny( OControlExchange::getControlPathFormatId() ) >>= aControlPathData )
            {
                DBG_ASSERT( aControlPathData.getLength() >= 2, "OControlTransferData::OControlTransferData: invalid data for the control path format!" );
                if ( aControlPathData.getLength() >= 2 )
                {
                    aControlPathData[0] >>= m_xFormsRoot;
                    aControlPathData[1] >>= m_aControlPaths;
                }
            }
            else
                DBG_ERROR( "OControlTransferData::OControlTransferData: invalid data for the control path format (2)!" );
        }
        if ( OControlExchange::hasHiddenControlModelsFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // sequence of models of hidden controls
            aExchangedData.GetAny( OControlExchange::getHiddenControlModelsFormatId() ) >>= m_aHiddenControlModels;
        }

        updateFormats( );
    }

    //--------------------------------------------------------------------
    static sal_Bool lcl_fillDataFlavorEx( SotFormatStringId nId, DataFlavorEx& _rFlavor )
    {
        _rFlavor.mnSotId = nId;
        return SotExchange::GetFormatDataFlavor( _rFlavor.mnSotId, _rFlavor );
    }

    //--------------------------------------------------------------------
    void OControlTransferData::updateFormats( )
    {
        m_aCurrentFormats.clear();
        m_aCurrentFormats.reserve( 3 );

        DataFlavorEx aFlavor;

        if ( m_aHiddenControlModels.getLength() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getHiddenControlModelsFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }

        if ( m_xFormsRoot.is() && m_aControlPaths.getLength() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getControlPathFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }

        if ( m_aSelectedEntries.size() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getFieldExchangeFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }
    }

    //--------------------------------------------------------------------
    void OControlTransferData::addSelectedEntry( SvLBoxEntry* _pEntry )
    {
        m_aSelectedEntries.push_back(_pEntry);
    }

    //--------------------------------------------------------------------
    void OControlTransferData::setFocusEntry( SvLBoxEntry* _pFocusEntry )
    {
        m_pFocusEntry = _pFocusEntry;
    }

    //------------------------------------------------------------------------
    void OControlTransferData::addHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqInterfaces)
    {
        m_aHiddenControlModels = seqInterfaces;
    }

    //------------------------------------------------------------------------
    void OControlTransferData::buildPathFormat(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot)
    {
        m_aControlPaths.realloc(0);

        sal_Int32 nEntryCount = m_aSelectedEntries.size();
        if (nEntryCount == 0)
            return;

        m_aControlPaths.realloc(nEntryCount);
        ::com::sun::star::uno::Sequence<sal_uInt32>* pAllPaths = m_aControlPaths.getArray();
        for (sal_Int32 i=0; i<nEntryCount; ++i)
        {
            // erst mal sammeln wir den Pfad in einem Array ein
            ::std::vector< sal_uInt32 > aCurrentPath;
            SvLBoxEntry* pCurrentEntry = m_aSelectedEntries[i];

            SvLBoxEntry* pLoop = pCurrentEntry;
            while (pLoop != pRoot)
            {
                aCurrentPath.push_back(pLoop->GetChildListPos());
                pLoop = pTreeBox->GetParent(pLoop);
                DBG_ASSERT((pLoop != NULL) || (pRoot == 0), "OControlTransferData::buildPathFormat: invalid root or entry !");
                    // pLoop == NULL heisst, dass ich am oberen Ende angelangt bin, dann sollte das Ganze abbrechen, was nur bei pRoot == NULL der Fall sein wird
            }

            // dann koennen wir ihn in die ::com::sun::star::uno::Sequence uebertragen
            ::com::sun::star::uno::Sequence<sal_uInt32>& rCurrentPath = pAllPaths[i];
            sal_Int32 nDepth = aCurrentPath.size();

            rCurrentPath.realloc(nDepth);
            sal_uInt32* pSeq = rCurrentPath.getArray();
            sal_Int32 j,k;
            for (j = nDepth - 1, k = 0; k<nDepth; --j, ++k)
                pSeq[j] = aCurrentPath[k];
        }
    }

    //------------------------------------------------------------------------
    void OControlTransferData::buildListFromPath(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot)
    {
        m_aSelectedEntries.clear();

        sal_Int32 nControls = m_aControlPaths.getLength();
        const ::com::sun::star::uno::Sequence<sal_uInt32>* pPaths = m_aControlPaths.getConstArray();
        for (sal_Int32 i=0; i<nControls; ++i)
        {
            sal_Int32 nThisPatLength = pPaths[i].getLength();
            const sal_uInt32* pThisPath = pPaths[i].getConstArray();
            SvLBoxEntry* pSearch = pRoot;
            for (sal_Int32 j=0; j<nThisPatLength; ++j)
                pSearch = pTreeBox->GetEntry(pSearch, pThisPath[j]);

            m_aSelectedEntries.push_back(pSearch);
        }
    }

    //====================================================================
    //= OControlExchange
    //====================================================================
    //--------------------------------------------------------------------
    OControlExchange::OControlExchange( )
    {
    }

    //--------------------------------------------------------------------
    sal_Bool OControlExchange::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat( _rFlavor );

        if ( getControlPathFormatId( ) == nFormatId )
        {
            // ugly. We have to pack all the info into one object
            Sequence< Any > aCompleteInfo( 2 );
            OSL_ENSURE( m_xFormsRoot.is(), "OLocalExchange::GetData: invalid forms root for this format!" );
            aCompleteInfo.getArray()[ 0 ] <<= m_xFormsRoot;
            aCompleteInfo.getArray()[ 1 ] <<= m_aControlPaths;

            SetAny( makeAny( aCompleteInfo ), _rFlavor );
        }
        else if ( getHiddenControlModelsFormatId() == nFormatId )
        {
            // just need to transfer the models
            SetAny( makeAny( m_aHiddenControlModels ), _rFlavor );
        }
        else
            return OLocalExchange::GetData( _rFlavor );

        return sal_True;
    }

    //--------------------------------------------------------------------
    void OControlExchange::AddSupportedFormats()
    {
        if (m_pFocusEntry && m_aSelectedEntries.size())
            AddFormat(getFieldExchangeFormatId());

        if (m_aControlPaths.getLength())
            AddFormat(getControlPathFormatId());

        if (m_aHiddenControlModels.getLength())
            AddFormat(getHiddenControlModelsFormatId());
    }

    //--------------------------------------------------------------------
    sal_uInt32 OControlExchange::getControlPathFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"svxform.ControlPathExchange\""));
            DBG_ASSERT((sal_uInt32)-1 != s_nFormat, "OControlExchange::getControlPathFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    //--------------------------------------------------------------------
    sal_uInt32 OControlExchange::getHiddenControlModelsFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"svxform.HiddenControlModelsExchange\""));
            DBG_ASSERT((sal_uInt32)-1 != s_nFormat, "OControlExchange::getHiddenControlModelsFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    //--------------------------------------------------------------------
    sal_uInt32 OControlExchange::getFieldExchangeFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"svxform.FieldNameExchange\""));
            DBG_ASSERT((sal_uInt32)-1 != s_nFormat, "OControlExchange::getFieldExchangeFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    //====================================================================
    //= OControlExchangeHelper
    //====================================================================
    OLocalExchange* OControlExchangeHelper::createExchange() const
    {
        return new OControlExchange;
    }

    //====================================================================
    //= OLocalExchangeHelper
    //====================================================================
    //--------------------------------------------------------------------
    OLocalExchangeHelper::OLocalExchangeHelper(Window* _pDragSource)
        :m_pDragSource(_pDragSource)
        ,m_pTransferable(NULL)
    {
    }

    //--------------------------------------------------------------------
    OLocalExchangeHelper::~OLocalExchangeHelper()
    {
        implReset();
    }

    //--------------------------------------------------------------------
    void OLocalExchangeHelper::startDrag( sal_Int8 nDragSourceActions )
    {
        DBG_ASSERT(m_pTransferable, "OLocalExchangeHelper::startDrag: not prepared!");
        m_pTransferable->startDrag( m_pDragSource, nDragSourceActions, OLocalExchange::GrantAccess() );
    }

    //--------------------------------------------------------------------
    void OLocalExchangeHelper::copyToClipboard( ) const
    {
        DBG_ASSERT( m_pTransferable, "OLocalExchangeHelper::copyToClipboard: not prepared!" );
        m_pTransferable->copyToClipboard( m_pDragSource, OLocalExchange::GrantAccess() );
    }

    //--------------------------------------------------------------------
    void OLocalExchangeHelper::implReset()
    {
        if (m_pTransferable)
        {
            m_pTransferable->setClipboardListener( Link() );
            m_pTransferable->release();
            m_pTransferable = NULL;
        }
    }

    //--------------------------------------------------------------------
    void OLocalExchangeHelper::prepareDrag( )
    {
        DBG_ASSERT(!m_pTransferable || !m_pTransferable->isDragging(), "OLocalExchangeHelper::prepareDrag: recursive DnD?");

        implReset();
        m_pTransferable = createExchange();
        m_pTransferable->acquire();
    }

//........................................................................
}
//........................................................................

