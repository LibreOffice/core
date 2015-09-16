/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "fmexch.hxx"

#include <sot/storage.hxx>
#include <svl/itempool.hxx>

#include <sot/formats.hxx>
#include <svtools/treelistbox.hxx>
#include "svtools/treelistentry.hxx"
#include <tools/diagnose_ex.h>



namespace svxform
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::datatransfer;

    OLocalExchange::OLocalExchange( )
        :m_bDragging( false )
        ,m_bClipboardOwner( false )
    {
    }


    void OLocalExchange::copyToClipboard( vcl::Window* _pWindow, const GrantAccess& )
    {
        if ( m_bClipboardOwner )
        {   // simulate a lostOwnership to notify parties interested in
            if ( m_aClipboardListener.IsSet() )
                m_aClipboardListener.Call( *this );
        }

        m_bClipboardOwner = true;
        CopyToClipboard( _pWindow );
    }


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
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            m_bClipboardOwner = false;
        }
    }


    void SAL_CALL OLocalExchange::lostOwnership( const Reference< clipboard::XClipboard >& _rxClipboard, const Reference< XTransferable >& _rxTrans ) throw(RuntimeException, std::exception)
    {
        TransferableHelper::implCallOwnLostOwnership( _rxClipboard, _rxTrans );
        m_bClipboardOwner = false;

        if ( m_aClipboardListener.IsSet() )
            m_aClipboardListener.Call( *this );
    }


    void OLocalExchange::startDrag( vcl::Window* _pWindow, sal_Int8 _nDragSourceActions, const GrantAccess& )
    {
        m_bDragging = true;
        StartDrag( _pWindow, _nDragSourceActions );
    }


    void OLocalExchange::DragFinished( sal_Int8 nDropAction )
    {
        TransferableHelper::DragFinished( nDropAction );
        m_bDragging = false;
    }


    bool OLocalExchange::hasFormat( const DataFlavorExVector& _rFormats, SotClipboardFormatId _nFormatId )
    {
        DataFlavorExVector::const_iterator aSearch;

        for ( aSearch = _rFormats.begin(); aSearch != _rFormats.end(); ++aSearch )
            if ( aSearch->mnSotId == _nFormatId )
                break;

        return aSearch != _rFormats.end();
    }


    bool OLocalExchange::GetData( const css::datatransfer::DataFlavor& /*_rFlavor*/, const OUString& /*rDestDoc*/ )
    {
        return false;   // do not have any formats by default
    }

    OControlTransferData::OControlTransferData( )
        :m_pFocusEntry( NULL )
    {
    }


    OControlTransferData::OControlTransferData( const Reference< XTransferable >& _rxTransferable )
        :m_pFocusEntry( NULL )
    {
        TransferableDataHelper aExchangedData( _rxTransferable );

        // try the formats we know
        if ( OControlExchange::hasControlPathFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // paths to the controls, relative to a root
            Sequence< Any > aControlPathData;
            if ( aExchangedData.GetAny(OControlExchange::getControlPathFormatId(), OUString()) >>= aControlPathData )
            {
                DBG_ASSERT( aControlPathData.getLength() >= 2, "OControlTransferData::OControlTransferData: invalid data for the control path format!" );
                if ( aControlPathData.getLength() >= 2 )
                {
                    aControlPathData[0] >>= m_xFormsRoot;
                    aControlPathData[1] >>= m_aControlPaths;
                }
            }
            else
            {
                OSL_FAIL( "OControlTransferData::OControlTransferData: invalid data for the control path format (2)!" );
            }
        }
        if ( OControlExchange::hasHiddenControlModelsFormat( aExchangedData.GetDataFlavorExVector() ) )
        {   // sequence of models of hidden controls
            aExchangedData.GetAny(OControlExchange::getHiddenControlModelsFormatId(), OUString()) >>= m_aHiddenControlModels;
        }

        updateFormats( );
    }


    static bool lcl_fillDataFlavorEx( SotClipboardFormatId nId, DataFlavorEx& _rFlavor )
    {
        _rFlavor.mnSotId = nId;
        return SotExchange::GetFormatDataFlavor( _rFlavor.mnSotId, _rFlavor );
    }


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

        if ( !m_aSelectedEntries.empty() )
        {
            if ( lcl_fillDataFlavorEx( OControlExchange::getFieldExchangeFormatId(), aFlavor ) )
                m_aCurrentFormats.push_back( aFlavor );
        }
    }


    size_t OControlTransferData::onEntryRemoved( SvTreeListEntry* _pEntry )
    {
        m_aSelectedEntries.erase( _pEntry );
        return m_aSelectedEntries.size();
    }


    void OControlTransferData::addSelectedEntry( SvTreeListEntry* _pEntry )
    {
        m_aSelectedEntries.insert( _pEntry );
    }


    void OControlTransferData::setFocusEntry( SvTreeListEntry* _pFocusEntry )
    {
        m_pFocusEntry = _pFocusEntry;
    }


    void OControlTransferData::addHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& seqInterfaces)
    {
        m_aHiddenControlModels = seqInterfaces;
    }


    void OControlTransferData::buildPathFormat(SvTreeListBox* pTreeBox, SvTreeListEntry* pRoot)
    {
        m_aControlPaths.realloc(0);

        sal_Int32 nEntryCount = m_aSelectedEntries.size();
        if (nEntryCount == 0)
            return;

        m_aControlPaths.realloc(nEntryCount);
        ::com::sun::star::uno::Sequence<sal_uInt32>* pAllPaths = m_aControlPaths.getArray();
        for (   ListBoxEntrySet::const_iterator loop = m_aSelectedEntries.begin();
                loop != m_aSelectedEntries.end();
                ++loop, ++pAllPaths
            )
        {
            // erst mal sammeln wir den Pfad in einem Array ein
            ::std::vector< sal_uInt32 > aCurrentPath;
            SvTreeListEntry* pCurrentEntry = *loop;

            SvTreeListEntry* pLoop = pCurrentEntry;
            while (pLoop != pRoot)
            {
                aCurrentPath.push_back(pLoop->GetChildListPos());
                pLoop = pTreeBox->GetParent(pLoop);
                DBG_ASSERT((pLoop != NULL) || (pRoot == 0), "OControlTransferData::buildPathFormat: invalid root or entry !");
                    // pLoop == NULL heisst, dass ich am oberen Ende angelangt bin, dann sollte das Ganze abbrechen, was nur bei pRoot == NULL der Fall sein wird
            }

            // dann koennen wir ihn in die ::com::sun::star::uno::Sequence uebertragen
            Sequence<sal_uInt32>& rCurrentPath = *pAllPaths;
            sal_Int32 nDepth = aCurrentPath.size();

            rCurrentPath.realloc(nDepth);
            sal_uInt32* pSeq = rCurrentPath.getArray();
            sal_Int32 j,k;
            for (j = nDepth - 1, k = 0; k<nDepth; --j, ++k)
                pSeq[j] = aCurrentPath[k];
        }
    }


    void OControlTransferData::buildListFromPath(SvTreeListBox* pTreeBox, SvTreeListEntry* pRoot)
    {
        ListBoxEntrySet aEmpty;
        m_aSelectedEntries.swap( aEmpty );

        sal_Int32 nControls = m_aControlPaths.getLength();
        const ::com::sun::star::uno::Sequence<sal_uInt32>* pPaths = m_aControlPaths.getConstArray();
        for (sal_Int32 i=0; i<nControls; ++i)
        {
            sal_Int32 nThisPatLength = pPaths[i].getLength();
            const sal_uInt32* pThisPath = pPaths[i].getConstArray();
            SvTreeListEntry* pSearch = pRoot;
            for (sal_Int32 j=0; j<nThisPatLength; ++j)
                pSearch = pTreeBox->GetEntry(pSearch, pThisPath[j]);

            m_aSelectedEntries.insert( pSearch );
        }
    }

    OControlExchange::OControlExchange( )
    {
    }


    bool OControlExchange::GetData( const DataFlavor& _rFlavor, const OUString& rDestDoc )
    {
        const SotClipboardFormatId nFormatId = SotExchange::GetFormat( _rFlavor );

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
            return OLocalExchange::GetData(_rFlavor, rDestDoc);

        return true;
    }


    void OControlExchange::AddSupportedFormats()
    {
        if (m_pFocusEntry && !m_aSelectedEntries.empty())
            AddFormat(getFieldExchangeFormatId());

        if (m_aControlPaths.getLength())
            AddFormat(getControlPathFormatId());

        if (m_aHiddenControlModels.getLength())
            AddFormat(getHiddenControlModelsFormatId());
    }


    SotClipboardFormatId OControlExchange::getControlPathFormatId()
    {
        static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
        if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"svxform.ControlPathExchange\""));
            DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getControlPathFormatId: bad exchange id!");
        }
        return s_nFormat;
    }


    SotClipboardFormatId OControlExchange::getHiddenControlModelsFormatId()
    {
        static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
        if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"svxform.HiddenControlModelsExchange\""));
            DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getHiddenControlModelsFormatId: bad exchange id!");
        }
        return s_nFormat;
    }


    SotClipboardFormatId OControlExchange::getFieldExchangeFormatId()
    {
        static SotClipboardFormatId s_nFormat = static_cast<SotClipboardFormatId>(-1);
        if (static_cast<SotClipboardFormatId>(-1) == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"svxform.FieldNameExchange\""));
            DBG_ASSERT(static_cast<SotClipboardFormatId>(-1) != s_nFormat, "OControlExchange::getFieldExchangeFormatId: bad exchange id!");
        }
        return s_nFormat;
    }


    //= OControlExchangeHelper

    OLocalExchange* OControlExchangeHelper::createExchange() const
    {
        return new OControlExchange;
    }

    OLocalExchangeHelper::OLocalExchangeHelper(vcl::Window* _pDragSource)
        :m_pDragSource(_pDragSource)
        ,m_pTransferable(NULL)
    {
    }


    OLocalExchangeHelper::~OLocalExchangeHelper()
    {
        implReset();
    }


    void OLocalExchangeHelper::startDrag( sal_Int8 nDragSourceActions )
    {
        DBG_ASSERT(m_pTransferable, "OLocalExchangeHelper::startDrag: not prepared!");
        m_pTransferable->startDrag( m_pDragSource, nDragSourceActions, OLocalExchange::GrantAccess() );
    }


    void OLocalExchangeHelper::copyToClipboard( ) const
    {
        DBG_ASSERT( m_pTransferable, "OLocalExchangeHelper::copyToClipboard: not prepared!" );
        m_pTransferable->copyToClipboard( m_pDragSource, OLocalExchange::GrantAccess() );
    }


    void OLocalExchangeHelper::implReset()
    {
        if (m_pTransferable)
        {
            m_pTransferable->setClipboardListener( Link<OLocalExchange&,void>() );
            m_pTransferable->release();
            m_pTransferable = NULL;
        }
    }


    void OLocalExchangeHelper::prepareDrag( )
    {
        DBG_ASSERT(!m_pTransferable || !m_pTransferable->isDragging(), "OLocalExchangeHelper::prepareDrag: recursive DnD?");

        implReset();
        m_pTransferable = createExchange();
        m_pTransferable->acquire();
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
