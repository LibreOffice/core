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

#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#define _CONNECTIVITY_ADO_AWRAPADOX_HXX_

#include <com/sun/star/beans/PropertyAttribute.hpp>

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


#include "ado_pre_sys_include.h"
#include <adoint.h>
#include <adoctint.h>
#include "ado_post_sys_include.h"


#include "ado/Aolewrap.hxx"
#include "ado/Aolevariant.hxx"
#include "ado/adoimp.hxx"
#include "ado/Awrapado.hxx"
#include "ado/WrapColumn.hxx"
#include "ado/WrapIndex.hxx"
#include "ado/WrapKey.hxx"
#include "ado/WrapTable.hxx"
#include "ado/WrapCatalog.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOView : public WpOLEBase<ADOView>
        {
        public:
            WpADOView(ADOView* pInt=NULL)   :   WpOLEBase<ADOView>(pInt){}
            WpADOView(const WpADOView& rhs){operator=(rhs);}

            inline WpADOView& operator=(const WpADOView& rhs)
                {WpOLEBase<ADOView>::operator=(rhs); return *this;}

            ::rtl::OUString get_Name() const;
            void get_Command(OLEVariant& _rVar) const;
            void put_Command(OLEVariant& _rVar);
        };

        class WpADOGroup : public WpOLEBase<ADOGroup>
        {
        public:
            WpADOGroup(ADOGroup* pInt=NULL) :   WpOLEBase<ADOGroup>(pInt){}
            WpADOGroup(const WpADOGroup& rhs){operator=(rhs);}

            inline WpADOGroup& operator=(const WpADOGroup& rhs)
                {WpOLEBase<ADOGroup>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void put_Name(const ::rtl::OUString& _rName);
            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType);
            sal_Bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights);
            WpADOUsers get_Users( );
        };

        class WpADOUser : public WpOLEBase<_ADOUser>
        {
        public:
            WpADOUser(_ADOUser* pInt=NULL)  :   WpOLEBase<_ADOUser>(pInt){}
            WpADOUser(const WpADOUser& rhs){operator=(rhs);}

            inline WpADOUser& operator=(const WpADOUser& rhs)
                {WpOLEBase<_ADOUser>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void put_Name(const ::rtl::OUString& _rName);
            sal_Bool ChangePassword(const ::rtl::OUString& _rPwd,const ::rtl::OUString& _rNewPwd);
            WpADOGroups get_Groups();
            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType);
            sal_Bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights);
        };
    }
}
#endif // _CONNECTIVITY_ADO_AWRAPADOX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
