/*************************************************************************
 *
 *  $RCSfile: Awrapadox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:25 $
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

#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#define _CONNECTIVITY_ADO_AWRAPADOX_HXX_

// Includes fuer ADO
#include <tools/prewin.h>
//#include <oledb.h>
//#include <objbase.h>
//#include <initguid.h>
//#include <mapinls.h>
//#include <ocidl.h>

#ifndef __User_FWD_DEFINED__
#define __User_FWD_DEFINED__
typedef struct _ADOUser User;
#endif  /* __User_FWD_DEFINED__ */

#ifndef __Group_FWD_DEFINED__
#define __Group_FWD_DEFINED__
typedef struct _ADOGroup Group;
#endif /* __Group_FWD_DEFINED__ */

#ifndef __Column_FWD_DEFINED__
#define __Column_FWD_DEFINED__
typedef struct _ADOColumn Column;
#endif  /* __Column_FWD_DEFINED__ */

#ifndef __Index_FWD_DEFINED__
#define __Index_FWD_DEFINED__
typedef struct _ADOIndex Index;
#endif /* __cplusplus */

#ifndef __Key_FWD_DEFINED__
#define __Key_FWD_DEFINED__
typedef struct _ADOKey Key;
#endif  /* __Key_FWD_DEFINED__ */

#ifndef __Table_FWD_DEFINED__
#define __Table_FWD_DEFINED__
typedef struct _ADOTable Table;
#endif  /* __Table_FWD_DEFINED__ */

#ifndef _ADOINT_H_
#include <adoint.h>
#endif

#ifndef _ADOCTINT_H_
#include <ado/adoctint.h>
#endif
#include <tools/postwin.h>

#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#include "ado/Aolewrap.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AOLEVARIANT_HXX_
#include "ado/Aolevariant.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADOIMP_HXX_
#include "ado/adoimp.hxx"
#endif
namespace connectivity
{
    namespace ado
    {
        class WpADOColumn : public WpOLEBase<_ADOColumn>
        {
        public:
            WpADOColumn(_ADOColumn* pInt=NULL)  :   WpOLEBase<_ADOColumn>(pInt){}

            void Create();

            inline WpADOColumn& operator=(const WpADOColumn& rhs)
                {WpOLEBase<_ADOColumn>::operator=(rhs); return *this;}

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            DataTypeEnum get_Type() const
            {
                DataTypeEnum eNum;
                pInterface->get_Type(&eNum);
                return eNum;
            }

            void put_Type(const DataTypeEnum& _eNum)
            {
                pInterface->put_Type(_eNum);
            }

            sal_Int32 get_Precision() const
            {
                sal_Int32 nPrec=0;
                pInterface->get_Precision(&nPrec);
                return nPrec;
            }

            void put_Precision(sal_Int32 _nPre)
            {
                pInterface->put_Precision(_nPre);
            }

            sal_Int32 get_NumericScale() const
            {
                sal_uInt8 nPrec=0;
                pInterface->get_NumericScale(&nPrec);
                return nPrec;
            }

            void put_NumericScale(sal_Int32 _nScale)
            {
                pInterface->put_NumericScale(_nScale);
            }

            SortOrderEnum get_SortOrder() const
            {
                SortOrderEnum nPrec;
                pInterface->get_SortOrder(&nPrec);
                return nPrec;
            }

            void put_SortOrder(SortOrderEnum _nScale)
            {
                pInterface->put_SortOrder(_nScale);
            }

            ColumnAttributesEnum get_Attributes() const
            {
                ColumnAttributesEnum eNum;
                pInterface->get_Attributes(&eNum);
                return eNum;
            }

            void put_Attributes(const ColumnAttributesEnum& _eNum)
            {
                pInterface->put_Attributes(_eNum);
            }

            ADOProperties* get_Properties() const
            {
                ADOProperties* pProps = NULL;
                pInterface->get_Properties(&pProps);
                return pProps;
            }
        };

        class WpADOKey : public WpOLEBase<ADOKey>
        {
        public:
            WpADOKey(ADOKey* pInt=NULL) :   WpOLEBase<ADOKey>(pInt){}

            inline WpADOKey& operator=(const WpADOKey& rhs)
                {WpOLEBase<ADOKey>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            KeyTypeEnum get_Type() const
            {
                KeyTypeEnum eNum;
                pInterface->get_Type(&eNum);
                return eNum;
            }

            void put_Type(const KeyTypeEnum& _eNum)
            {
                pInterface->put_Type(_eNum);
            }

            ::rtl::OUString get_RelatedTable() const
            {
                BSTR aBSTR;
                pInterface->get_RelatedTable(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_RelatedTable(const ::rtl::OUString& _rName)
            {
                pInterface->put_RelatedTable(SysAllocString(_rName.getStr()));
            }

            RuleEnum get_DeleteRule() const
            {
                RuleEnum eNum;
                pInterface->get_DeleteRule(&eNum);
                return eNum;
            }

            void put_DeleteRule(const RuleEnum& _eNum)
            {
                pInterface->put_DeleteRule(_eNum);
            }

            RuleEnum get_UpdateRule() const
            {
                RuleEnum eNum;
                pInterface->get_UpdateRule(&eNum);
                return eNum;
            }

            void put_UpdateRule(const RuleEnum& _eNum)
            {
                pInterface->put_UpdateRule(_eNum);
            }

            ADOColumns* get_Columns() const
            {
                ADOColumns* pCols = NULL;
                pInterface->get_Columns(&pCols);
                return pCols;
            }
        };

        class WpADOIndex : public WpOLEBase<_ADOIndex>
        {
        public:
            WpADOIndex(_ADOIndex* pInt=NULL)    :   WpOLEBase<_ADOIndex>(pInt){}

            inline WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            sal_Bool get_Clustered() const
            {
                VARIANT_BOOL eNum;
                pInterface->get_Clustered(&eNum);
                return eNum == VARIANT_TRUE;
            }

            void put_Clustered(sal_Bool _b)
            {
                pInterface->put_Clustered(_b ? VARIANT_TRUE : VARIANT_FALSE);
            }

            sal_Bool get_Unique() const
            {
                VARIANT_BOOL eNum;
                pInterface->get_Unique(&eNum);
                return eNum == VARIANT_TRUE;
            }

            void put_Unique(sal_Bool _b)
            {
                pInterface->put_Unique(_b ? VARIANT_TRUE : VARIANT_FALSE);
            }

            sal_Bool get_PrimaryKey() const
            {
                VARIANT_BOOL eNum;
                pInterface->get_PrimaryKey(&eNum);
                return eNum == VARIANT_TRUE;
            }

            void put_PrimaryKey(sal_Bool _b)
            {
                pInterface->put_PrimaryKey(_b ? VARIANT_TRUE : VARIANT_FALSE);
            }

            ADOColumns* get_Columns() const
            {
                ADOColumns* pCols = NULL;
                pInterface->get_Columns(&pCols);
                return pCols;
            }
        };

        class WpADOCatalog : public WpOLEBase<_ADOCatalog>
        {
        public:
            WpADOCatalog(_ADOCatalog* pInt = NULL)  :   WpOLEBase<_ADOCatalog>(pInt){}

            inline WpADOCatalog& operator=(const WpADOCatalog& rhs)
                {WpOLEBase<_ADOCatalog>::operator=(rhs); return *this;}

            ::rtl::OUString GetObjectOwner(const ::rtl::OUString& _rName, ObjectTypeEnum _eNum);

            void putref_ActiveConnection(IDispatch* pCon)
            {
                pInterface->putref_ActiveConnection(pCon);
            }

            ADOTables* get_Tables()
            {
                ADOTables* pRet = NULL;
                pInterface->get_Tables(&pRet);
                return pRet;
            }

            ADOViews* get_Views()
            {
                ADOViews* pRet = NULL;
                pInterface->get_Views(&pRet);
                return pRet;
            }

            ADOGroups* get_Groups()
            {
                ADOGroups* pRet = NULL;
                pInterface->get_Groups(&pRet);
                return pRet;
            }

            ADOUsers* get_Users()
            {
                ADOUsers* pRet = NULL;
                pInterface->get_Users(&pRet);
                return pRet;
            }

            ADOProcedures* get_Procedures()
            {
                ADOProcedures* pRet = NULL;
                pInterface->get_Procedures(&pRet);
                return pRet;
            }

            void Create();
        };

        class WpADOTable : public WpOLEBase<_ADOTable>
        {
        public:
            WpADOTable(_ADOTable* pInt=NULL)    :   WpOLEBase<_ADOTable>(pInt){}

            inline WpADOTable& operator=(const WpADOTable& rhs)
                {WpOLEBase<_ADOTable>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            ::rtl::OUString get_Type() const
            {
                BSTR aBSTR;
                pInterface->get_Type(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            ADOColumns* get_Columns() const
            {
                ADOColumns* pCols = NULL;
                pInterface->get_Columns(&pCols);
                return pCols;
            }

            ADOIndexes* get_Indexes() const
            {
                ADOIndexes* pCols = NULL;
                pInterface->get_Indexes(&pCols);
                return pCols;
            }

            ADOKeys* get_Keys() const
            {
                ADOKeys* pCols = NULL;
                pInterface->get_Keys(&pCols);
                return pCols;
            }

            WpADOCatalog get_ParentCatalog() const
            {
                ADOCatalog* pCat = NULL;
                pInterface->get_ParentCatalog(&pCat);
                return WpADOCatalog(pCat);
            }

            ADOProperties* get_Properties() const
            {
                ADOProperties* pProps = NULL;
                pInterface->get_Properties(&pProps);
                return pProps;
            }
        };

        class WpADOView : public WpOLEBase<ADOView>
        {
        public:
            WpADOView(ADOView* pInt=NULL)   :   WpOLEBase<ADOView>(pInt){}

            inline WpADOView& operator=(const WpADOView& rhs)
                {WpOLEBase<ADOView>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void get_Command(OLEVariant& _rVar) const
            {
                pInterface->get_Command(&_rVar);
            }

            void put_Command(OLEVariant& _rVar)
            {
                pInterface->put_Command(_rVar);
            }
        };

        class WpADOGroup : public WpOLEBase<_ADOGroup>
        {
        public:
            WpADOGroup(_ADOGroup* pInt=NULL)    :   WpOLEBase<_ADOGroup>(pInt){}

            inline WpADOGroup& operator=(const WpADOGroup& rhs)
                {WpOLEBase<_ADOGroup>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType)
            {
                RightsEnum Rights;
                OLEVariant ObjectTypeId;
                ObjectTypeId.setNoArg();
                pInterface->GetPermissions(Name,ObjectType,ObjectTypeId,&Rights);
                return Rights;
            }

            sal_Bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights)
            {
                OLEVariant ObjectTypeId;
                ObjectTypeId.setNoArg();
                return SUCCEEDED(pInterface->SetPermissions(Name,ObjectType,Action,Rights,adInheritNone,ObjectTypeId));
            }

            ADOUsers* get_Users( )
            {
                ADOUsers* pRet = NULL;
                pInterface->get_Users( &pRet);
                return pRet;
            }
        };

        class WpADOUser : public WpOLEBase<_ADOUser>
        {
        public:
            WpADOUser(_ADOUser* pInt=NULL)  :   WpOLEBase<_ADOUser>(pInt){}

            inline WpADOUser& operator=(const WpADOUser& rhs)
                {WpOLEBase<_ADOUser>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                BSTR aBSTR;
                pInterface->get_Name(&aBSTR);
                rtl::OUString sRetStr((sal_Unicode*)aBSTR);
                SysFreeString(aBSTR);
                return sRetStr;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                pInterface->put_Name(SysAllocString(_rName.getStr()));
            }

            sal_Bool ChangePassword(const ::rtl::OUString& _rPwd,const ::rtl::OUString& _rNewPwd)
            {
                return SUCCEEDED(pInterface->ChangePassword(SysAllocString(_rPwd.getStr()),SysAllocString(_rNewPwd.getStr())));
            }

            ADOGroups* get_Groups()
            {
                ADOGroups* pRet = NULL;
                pInterface->get_Groups(&pRet);
                return pRet;
            }

            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType)
            {
                RightsEnum Rights;
                OLEVariant ObjectTypeId;
                ObjectTypeId.setNoArg();
                pInterface->GetPermissions(Name,ObjectType,ObjectTypeId,&Rights);
                return Rights;
            }

            sal_Bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights)
            {
                OLEVariant ObjectTypeId;
                ObjectTypeId.setNoArg();
                return SUCCEEDED(pInterface->SetPermissions(Name,ObjectType,Action,Rights,adInheritNone,ObjectTypeId));
            }
        };
    }
}
#endif // _CONNECTIVITY_ADO_AWRAPADOX_HXX_

