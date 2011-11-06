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


#ifndef DBAUI_TABLEFIELDCONTROL_HXX
#define DBAUI_TABLEFIELDCONTROL_HXX

#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif

namespace dbaui
{
    class OTableEditorCtrl;
    class OTableDesignHelpBar;
    //==================================================================
    // OTableFieldControl
    //==================================================================
    class OTableFieldControl : public OFieldDescControl
    {
        OTableEditorCtrl*   GetCtrl() const;
    protected:
        virtual void        ActivateAggregate( EControlType eType );
        virtual void        DeactivateAggregate( EControlType eType );
        // Sind von den abgeleiteten Klassen zu impl.
        virtual void        CellModified(long nRow, sal_uInt16 nColId );
        virtual sal_Bool        IsReadOnly();
        virtual void        SetModified(sal_Bool bModified);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    GetFormatter() const;

        virtual ::com::sun::star::lang::Locale  GetLocale() const;

        virtual TOTypeInfoSP                    getTypeInfo(sal_Int32 _nPos);
        virtual const OTypeInfoMap*             getTypeInfo() const;
        virtual sal_Bool                        isAutoIncrementValueEnabled() const;
        virtual ::rtl::OUString                 getAutoIncrementValue() const;

    public:
        OTableFieldControl( Window* pParent, OTableDesignHelpBar* pHelpBar);

        String BoolStringPersistent(const String& rUIString) const { return OFieldDescControl::BoolStringPersistent(rUIString); }
        String BoolStringUI(const String& rPersistentString) const { return OFieldDescControl::BoolStringUI(rPersistentString); }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection();
    };
}
#endif // DBAUI_TABLEFIELDCONTROL_HXX


