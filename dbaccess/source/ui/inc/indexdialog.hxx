/*************************************************************************
 *
 *  $RCSfile: indexdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-19 06:02:09 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXDIALOG_HXX_
#define _DBAUI_INDEXDIALOG_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _DBAUI_INDEXES_HXX_
#include "indexes.hxx"
#endif

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

    protected:
        virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );
    };

    //==================================================================
    //= DbaIndexDialog
    //==================================================================
    class IndexFieldsControl;
    class OIndexCollection;
    class DbaIndexDialog : public ModalDialog
    {
    protected:
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

        OIndexCollection*       m_pIndexes;
        SvLBoxEntry*            m_pPreviousSelection;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;

    public:
        DbaIndexDialog(
            Window* _pParent,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rFieldNames,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            );
        ~DbaIndexDialog();

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

    private:
        void OnNewIndex();
        void OnDropIndex();
        void OnRenameIndex();
        void OnSaveIndex();
        void OnResetIndex();

        sal_Bool implCommit(SvLBoxEntry* _pEntry);
        sal_Bool implSaveModified(sal_Bool _bPlausibility = sal_True);
        sal_Bool implCommitPreviouslySelected();

        sal_Bool implDropIndex(SvLBoxEntry* _pEntry, sal_Bool _bRemoveFromCollection);

        sal_Bool implCheckPlausibility(const ConstIndexesIterator& _rPos);
    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXDIALOG_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/03/16 16:21:52  fs
 *  initial checkin - index design related classes
 *
 *
 *  Revision 1.0 07.03.01 11:51:22  fs
 ************************************************************************/

