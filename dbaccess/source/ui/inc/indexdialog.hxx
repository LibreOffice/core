/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXDIALOG_HXX_
#define _DBAUI_INDEXDIALOG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/svtreebx.hxx>
#include <unotools/viewoptions.hxx>
#include "indexes.hxx"
#include "ToolBoxHelper.hxx"

//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    //= DbaIndexList
    //==================================================================
    class DbaIndexList : public SvTreeListBox
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        Link        m_aSelectHdl;
        Link        m_aEndEditHdl;
        sal_Bool    m_bSuspendSelectHdl;

    public:
        DbaIndexList(Window* _pParent, const ResId& _rId);

        void SetSelectHdl(const Link& _rHdl) { m_aSelectHdl = _rHdl; }
        Link GetSelectHdl() const { return m_aSelectHdl; }

        void SetEndEditHdl(const Link& _rHdl) { m_aEndEditHdl = _rHdl; }
        Link GetEndEditHdl() const { return m_aEndEditHdl; }

        virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect );

        void enableSelectHandler();
        void disableSelectHandler();

        void SelectNoHandlerCall( SvLBoxEntry* pEntry );

        inline void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection)
        {
             m_xConnection = _rxConnection;
        }

    protected:
        virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

    private:
        using SvTreeListBox::Select;
    };

    //==================================================================
    //= DbaIndexDialog
    //==================================================================
    class IndexFieldsControl;
    class OIndexCollection;
    class DbaIndexDialog :  public ModalDialog,
                            public OToolBoxHelper
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
        SvtViewOptions          m_aGeometrySettings;

        ToolBox                 m_aActions;
        DbaIndexList            m_aIndexes;
        FixedLine               m_aIndexDetails;
        FixedText               m_aDescriptionLabel;
        FixedText               m_aDescription;
        CheckBox                m_aUnique;
        FixedText               m_aFieldsLabel;
        IndexFieldsControl*     m_pFields;
        PushButton              m_aClose;
        HelpButton              m_aHelp;

        OIndexCollection*       m_pIndexes;
        SvLBoxEntry*            m_pPreviousSelection;
        sal_Bool                m_bEditAgain;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;
    public:
        DbaIndexDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rFieldNames,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            sal_Int32 _nMaxColumnsInIndex
            );
        virtual ~DbaIndexDialog();

        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

        /** will be called whenthe id of the image list is needed.
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eBitmapSet,sal_Bool _bHiContast) const;

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& _rDiff);

    protected:
        void fillIndexList();
        void updateToolbox();
        void updateControls(const SvLBoxEntry* _pEntry);

    protected:
        DECL_LINK( OnIndexSelected, DbaIndexList* );
        DECL_LINK( OnIndexAction, ToolBox* );
        DECL_LINK( OnEntryEdited, SvLBoxEntry* );
        DECL_LINK( OnModified, void* );
        DECL_LINK( OnCloseDialog, void* );

        DECL_LINK( OnEditIndexAgain, SvLBoxEntry* );

    private:
        void OnNewIndex();
        void OnDropIndex(sal_Bool _bConfirm = sal_True);
        void OnRenameIndex();
        void OnSaveIndex();
        void OnResetIndex();

        sal_Bool implCommit(SvLBoxEntry* _pEntry);
        sal_Bool implSaveModified(sal_Bool _bPlausibility = sal_True);
        sal_Bool implCommitPreviouslySelected();

        sal_Bool implDropIndex(SvLBoxEntry* _pEntry, sal_Bool _bRemoveFromCollection);

        sal_Bool implCheckPlausibility(const ConstIndexesIterator& _rPos);

        /** checks if the controls have to be replaced and moved.
        */
        void checkControls();
    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
