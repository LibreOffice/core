/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            WpADOView(const WpADOView& rhs) : WpOLEBase<ADOView>(rhs) {}

            inline WpADOView& operator=(const WpADOView& rhs)
                {WpOLEBase<ADOView>::operator=(rhs); return *this;}

            OUString get_Name() const;
            void get_Command(OLEVariant& _rVar) const;
            void put_Command(OLEVariant& _rVar);
        };

        class WpADOGroup : public WpOLEBase<ADOGroup>
        {
        public:
            WpADOGroup(ADOGroup* pInt=NULL) :   WpOLEBase<ADOGroup>(pInt){}
            WpADOGroup(const WpADOGroup& rhs) : WpOLEBase<ADOGroup>(rhs) {}

            inline WpADOGroup& operator=(const WpADOGroup& rhs)
                {WpOLEBase<ADOGroup>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void put_Name(const OUString& _rName);
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
            WpADOUser(const WpADOUser& rhs) : WpOLEBase<_ADOUser>(rhs) {}

            inline WpADOUser& operator=(const WpADOUser& rhs)
                {WpOLEBase<_ADOUser>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void put_Name(const OUString& _rName);
            sal_Bool ChangePassword(const OUString& _rPwd,const OUString& _rNewPwd);
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
