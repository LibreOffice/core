/*************************************************************************
 *
 *  $RCSfile: WTypeSelect.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 14:58:35 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_WIZ_TYPESELECT_HXX
#define DBAUI_WIZ_TYPESELECT_HXX

#ifndef DBAUI_WIZ_TABBPAGE_HXX
#include "WTabPage.hxx"
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

namespace dbaui
{
    class OTableDesignHelpBar;
    // =============================================================================================
    // OWizTypeSelectControl
    // =============================================================================================
    class OWizTypeSelectControl : public OFieldDescControl
    {
    protected:
        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );

        virtual void        CellModified(long nRow, sal_uInt16 nColId );
        virtual void        SetModified(sal_Bool bModified);

        virtual ::com::sun::star::lang::Locale  GetLocale() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter();
        virtual const OTypeInfo*    getTypeInfo(sal_Int32 _nPos);
        virtual const OTypeInfoMap* getTypeInfo() const;

    public:
        OWizTypeSelectControl(Window* pParent, OTableDesignHelpBar* pHelpBar=NULL)
            : OFieldDescControl(pParent,pHelpBar) { };
        OWizTypeSelectControl(Window* pParent, const ResId& rResId,OTableDesignHelpBar* pHelpBar=NULL)
            : OFieldDescControl(pParent,rResId,pHelpBar) { };
        ~OWizTypeSelectControl();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
    };


    // ========================================================
    // Wizard Page: OWizTypeSelectList
    // definiert nur das ::com::sun::star::ucb::Command f"ur das Contextmenu
    // ========================================================
    class OWizTypeSelectList : public MultiListBox
    {
        sal_Bool                m_bPKey;
        sal_Bool                IsPrimaryKeyAllowed() const;
    protected:
        virtual long            PreNotify( NotifyEvent& rNEvt );
    public:
        OWizTypeSelectList( Window* pParent, WinBits nStyle = WB_BORDER ) : MultiListBox(pParent,nStyle) {};
        OWizTypeSelectList( Window* pParent, const ResId& rResId ) : MultiListBox(pParent,rResId) {};
        void                    SetPKey(sal_Bool bPKey) { m_bPKey = bPKey; }
    };
    // ========================================================
    // Wizard Page: OWizTypeSelect
    // Dient als Basis Klasse fuer unterschiedliche Kopiereigenschaften
    // FillColumnList wird aufgerufen, wenn der Button AUTO ausgeloest wird.
    // ========================================================
    class OWizTypeSelect : public OWizardPage
    {
        friend class OWizTypeSelectControl;
        friend class OWizTypeSelectList;

        DECL_LINK( ColumnSelectHdl, MultiListBox* );
        DECL_LINK( ButtonClickHdl, Button * );

    protected:
        OWizTypeSelectList      m_lbColumnNames;
        GroupBox                m_gpColumns;
        OWizTypeSelectControl   m_aTypeControl;
        GroupBox                m_gpAutoType;
        FixedText               m_ftAuto;
        NumericField            m_etAuto;
        PushButton              m_pbAuto;

        Image                   m_imgPKey;

        virtual void            fillColumnList(sal_uInt32 nRows) = 0;
        void                    EnableAuto(sal_Bool bEnable);
    public:
        virtual void            Reset ( );
        virtual void            ActivatePage( );
        virtual void            Resize();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const;

        OWizTypeSelect(Window* pParent);
        virtual ~OWizTypeSelect();

    };
}
#endif // DBAUI_WIZ_TYPESELECT_HXX



