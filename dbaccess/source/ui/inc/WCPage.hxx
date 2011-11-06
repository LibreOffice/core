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


#ifndef DBAUI_WIZARD_CPAGE_HXX
#define DBAUI_WIZARD_CPAGE_HXX

#include "WTabPage.hxx"

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

namespace dbaui
{
    class OWizColumnSelect;
    class OWizNormalExtend;
    //========================================================================
    class OCopyTable : public OWizardPage
    {
    protected:
        FixedText                               m_ftTableName;
        Edit                                    m_edTableName;
        FixedLine                               m_aFL_Options;
        RadioButton                             m_aRB_DefData;
        RadioButton                             m_aRB_Def;
        RadioButton                             m_aRB_View;
        RadioButton                             m_aRB_AppendData;
        CheckBox                                m_aCB_UseHeaderLine;
        CheckBox                                m_aCB_PrimaryColumn;
        FixedText                               m_aFT_KeyName;
        Edit                                    m_edKeyName;
        sal_Int16                               m_nOldOperation;

        OWizColumnSelect*                       m_pPage2;
        OWizNormalExtend*                       m_pPage3;

        sal_Bool                                    m_bPKeyAllowed;
        sal_Bool                                    m_bUseHeaderAllowed;


        DECL_LINK( AppendDataClickHdl, Button* );
        DECL_LINK( RadioChangeHdl, Button* );
        DECL_LINK( KeyClickHdl, Button* );

        sal_Bool checkAppendData();
        //--------dyf add
        void SetAppendDataRadio();
        //--------add end

    public:
        virtual void            Reset();
        virtual void            ActivatePage();
        virtual sal_Bool            LeavePage();
        virtual String          GetTitle() const ;

        OCopyTable( Window * pParent );
        virtual ~OCopyTable();

        inline sal_Bool IsOptionDefData()       const { return m_aRB_DefData.IsChecked(); }
        inline sal_Bool IsOptionDef()           const { return m_aRB_Def.IsChecked(); }
        inline sal_Bool IsOptionAppendData()    const { return m_aRB_AppendData.IsChecked(); }
        inline sal_Bool IsOptionView()          const { return m_aRB_View.IsChecked(); }
        inline sal_Bool UseHeaderLine()         const { return m_aCB_UseHeaderLine.IsChecked(); }
        String      GetKeyName()            const { return m_edKeyName.GetText(); }

        void setCreateStyleAction();
        inline void disallowViews()
        {
            m_aRB_View.Disable();
        }
        inline void disallowUseHeaderLine()
        {
            m_bUseHeaderAllowed = sal_False;
            m_aCB_UseHeaderLine.Disable();
        }

        void setCreatePrimaryKey( bool _bDoCreate, const ::rtl::OUString& _rSuggestedName );
    };
}
#endif // DBAUI_WIZARD_CPAGE_HXX


