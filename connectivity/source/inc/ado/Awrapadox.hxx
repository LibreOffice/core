/*************************************************************************
 *
 *  $RCSfile: Awrapadox.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-27 11:38:26 $
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
            WpADOColumn(const WpADOColumn& rhs){operator=(rhs);}

            void Create();

            inline WpADOColumn& operator=(const WpADOColumn& rhs)
                {WpOLEBase<_ADOColumn>::operator=(rhs); return *this;}

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }
            ::rtl::OUString get_RelatedColumn() const
            {
                OLEString aBSTR;
                pInterface->get_RelatedColumn(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));
            }
            void put_RelatedColumn(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedColumn(bstr));
            }

            DataTypeEnum get_Type() const
            {
                DataTypeEnum eNum = adVarChar;
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

            void put_NumericScale(sal_Int8 _nScale)
            {
                pInterface->put_NumericScale(_nScale);
            }

            SortOrderEnum get_SortOrder() const
            {
                SortOrderEnum nPrec=adSortAscending;
                pInterface->get_SortOrder(&nPrec);
                return nPrec;
            }

            void put_SortOrder(SortOrderEnum _nScale)
            {
                pInterface->put_SortOrder(_nScale);
            }

            ColumnAttributesEnum get_Attributes() const
            {
                ColumnAttributesEnum eNum=adColNullable;
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
            WpADOKey(const WpADOKey& rhs){operator=(rhs);}

            inline WpADOKey& operator=(const WpADOKey& rhs)
                {WpOLEBase<ADOKey>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

            }

            KeyTypeEnum get_Type() const
            {
                KeyTypeEnum eNum=adKeyPrimary;
                pInterface->get_Type(&eNum);
                return eNum;
            }

            void put_Type(const KeyTypeEnum& _eNum)
            {
                pInterface->put_Type(_eNum);
            }

            ::rtl::OUString get_RelatedTable() const
            {
                OLEString aBSTR;
                pInterface->get_RelatedTable(&aBSTR);
                return aBSTR;
            }

            void put_RelatedTable(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_RelatedTable(bstr));

            }

            RuleEnum get_DeleteRule() const
            {
                RuleEnum eNum = adRINone;
                pInterface->get_DeleteRule(&eNum);
                return eNum;
            }

            void put_DeleteRule(const RuleEnum& _eNum)
            {
                pInterface->put_DeleteRule(_eNum);
            }

            RuleEnum get_UpdateRule() const
            {
                RuleEnum eNum = adRINone;
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
            WpADOIndex(const WpADOIndex& rhs){operator=(rhs);}

            inline WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

            }

            sal_Bool get_Clustered() const
            {
                VARIANT_BOOL eNum = VARIANT_FALSE;
                pInterface->get_Clustered(&eNum);
                return eNum == VARIANT_TRUE;
            }

            void put_Clustered(sal_Bool _b)
            {
                pInterface->put_Clustered(_b ? VARIANT_TRUE : VARIANT_FALSE);
            }

            sal_Bool get_Unique() const
            {
                VARIANT_BOOL eNum = VARIANT_FALSE;
                pInterface->get_Unique(&eNum);
                return eNum == VARIANT_TRUE;
            }

            void put_Unique(sal_Bool _b)
            {
                pInterface->put_Unique(_b ? VARIANT_TRUE : VARIANT_FALSE);
            }

            sal_Bool get_PrimaryKey() const
            {
                VARIANT_BOOL eNum = VARIANT_FALSE;
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
            WpADOCatalog(const WpADOCatalog& rhs){operator=(rhs);}

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
            WpADOTable(const WpADOTable& rhs){operator=(rhs);}

            inline WpADOTable& operator=(const WpADOTable& rhs)
                {WpOLEBase<_ADOTable>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

            }

            ::rtl::OUString get_Type() const
            {
                OLEString aBSTR;
                pInterface->get_Type(&aBSTR);
                return aBSTR;
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
            WpADOView(const WpADOView& rhs){operator=(rhs);}

            inline WpADOView& operator=(const WpADOView& rhs)
                {WpOLEBase<ADOView>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
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
            WpADOGroup(const WpADOGroup& rhs){operator=(rhs);}

            inline WpADOGroup& operator=(const WpADOGroup& rhs)
                {WpOLEBase<_ADOGroup>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

            }

            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType)
            {
                RightsEnum Rights=adRightNone;
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
            WpADOUser(const WpADOUser& rhs){operator=(rhs);}

            inline WpADOUser& operator=(const WpADOUser& rhs)
                {WpOLEBase<_ADOUser>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const
            {
                OLEString aBSTR;
                pInterface->get_Name(&aBSTR);
                return aBSTR;
            }

            void put_Name(const ::rtl::OUString& _rName)
            {
                OLEString bstr(_rName);
                sal_Bool bErg = SUCCEEDED(pInterface->put_Name(bstr));

            }

            sal_Bool ChangePassword(const ::rtl::OUString& _rPwd,const ::rtl::OUString& _rNewPwd)
            {
                OLEString sStr1(_rPwd);
                OLEString sStr2(_rNewPwd);
                sal_Bool bErg = SUCCEEDED(pInterface->ChangePassword(sStr1,sStr2));
                return bErg;
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
                RightsEnum Rights=adRightNone;
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

