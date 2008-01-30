/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WTypeSelect.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:49:15 $
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
#ifndef DBAUI_WIZ_TYPESELECT_HXX
#define DBAUI_WIZ_TYPESELECT_HXX

#include "FieldDescControl.hxx"
#include "TypeInfo.hxx"
#include "WTabPage.hxx"

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

class SvStream;
class SvParser;
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

        virtual ::com::sun::star::lang::Locale  GetLocale() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const;
        virtual TOTypeInfoSP        getTypeInfo(sal_Int32 _nPos);
        virtual const OTypeInfoMap* getTypeInfo() const;
        virtual sal_Bool            isAutoIncrementValueEnabled() const;
        virtual ::rtl::OUString     getAutoIncrementValue() const;

    public:
        OWizTypeSelectControl(Window* pParent, const ResId& rResId,OTableDesignHelpBar* pHelpBar=NULL);
        virtual ~OWizTypeSelectControl();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection();
    };


    // ========================================================
    // Wizard Page: OWizTypeSelectList
    // definiert nur das ::com::sun::star::ucb::Command f"ur das Contextmenu
    // ========================================================
    class OWizTypeSelectList : public MultiListBox
    {
        sal_Bool                m_bPKey;
        sal_Bool                IsPrimaryKeyAllowed() const;
        void                    setPrimaryKey(  OFieldDescription* _pFieldDescr,
                                                sal_uInt16 _nPos,
                                                sal_Bool _bSet=sal_False);
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
        FixedLine               m_flColumns;
        OWizTypeSelectControl   m_aTypeControl;
        FixedLine               m_flAutoType;
        FixedText               m_ftAuto;
        NumericField            m_etAuto;
        PushButton              m_pbAuto;

        Image                   m_imgPKey;
        SvStream*               m_pParserStream; // stream to read the tokens from or NULL
        ::rtl::OUString         m_sAutoIncrementValue;
        sal_Int32               m_nDisplayRow;
        sal_Bool                m_bAutoIncrementEnabled;
        sal_Bool                m_bDuplicateName;

        void                    fillColumnList(sal_uInt32 nRows);
        virtual SvParser*       createReader(sal_Int32 _nRows) = 0;

        void                    EnableAuto(sal_Bool bEnable);
    public:
        virtual void            Reset ( );
        virtual void            ActivatePage( );
        virtual void            Resize();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const;

        OWizTypeSelect(Window* pParent, SvStream* _pStream = NULL );
        virtual ~OWizTypeSelect();

        inline void setDisplayRow(sal_Int32 _nRow) { m_nDisplayRow = _nRow - 1; }
        inline void setDuplicateName(sal_Bool _bDuplicateName) { m_bDuplicateName = _bDuplicateName; }
    };

    // ========================================================
    typedef OWizTypeSelect* (*TypeSelectionPageFactory)( Window*, SvStream& );
}
#endif // DBAUI_WIZ_TYPESELECT_HXX



