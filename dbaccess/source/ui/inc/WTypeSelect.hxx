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



