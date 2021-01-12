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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#ifndef __User_FWD_DEFINED__
#define __User_FWD_DEFINED__
typedef struct _ADOUser User;
#endif

#ifndef __Group_FWD_DEFINED__
#define __Group_FWD_DEFINED__
typedef struct _ADOGroup Group;
#endif

#ifndef __Column_FWD_DEFINED__
#define __Column_FWD_DEFINED__
typedef struct _ADOColumn Column;
#endif

#ifndef __Index_FWD_DEFINED__
#define __Index_FWD_DEFINED__
typedef struct _ADOIndex Index;
#endif

#ifndef __Key_FWD_DEFINED__
#define __Key_FWD_DEFINED__
typedef struct _ADOKey Key;
#endif

#ifndef __Table_FWD_DEFINED__
#define __Table_FWD_DEFINED__
typedef struct _ADOTable Table;
#endif


#include <adoint.h>
#include <adoctint.h>


#include <ado/Aolewrap.hxx>
#include <ado/Aolevariant.hxx>
#include <ado/adoimp.hxx>
#include <ado/Awrapado.hxx>
#include <ado/WrapColumn.hxx>
#include <ado/WrapIndex.hxx>
#include <ado/WrapKey.hxx>
#include <ado/WrapTable.hxx>
#include <ado/WrapCatalog.hxx>

namespace connectivity::ado
{
        class WpADOView : public WpOLEBase<ADOView>
        {
        public:
            WpADOView(ADOView* pInt=nullptr)   :   WpOLEBase<ADOView>(pInt){}
            WpADOView(const WpADOView& rhs) : WpOLEBase<ADOView>(rhs) {}

            WpADOView& operator=(const WpADOView& rhs)
                {WpOLEBase<ADOView>::operator=(rhs); return *this;}

            OUString get_Name() const;
            void get_Command(OLEVariant& _rVar) const;
            void put_Command(OLEVariant const & _rVar);
        };

        class WpADOGroup : public WpOLEBase<ADOGroup>
        {
        public:
            WpADOGroup(ADOGroup* pInt=nullptr) :   WpOLEBase<ADOGroup>(pInt){}
            WpADOGroup(const WpADOGroup& rhs) : WpOLEBase<ADOGroup>(rhs) {}

            WpADOGroup& operator=(const WpADOGroup& rhs)
                {WpOLEBase<ADOGroup>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void put_Name(std::u16string_view _rName);
            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType);
            bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights);
            WpADOUsers get_Users( );
        };

        class WpADOUser : public WpOLEBase<_ADOUser>
        {
        public:
            WpADOUser(_ADOUser* pInt=nullptr)  :   WpOLEBase<_ADOUser>(pInt){}
            WpADOUser(const WpADOUser& rhs) : WpOLEBase<_ADOUser>(rhs) {}

            WpADOUser& operator=(const WpADOUser& rhs)
                {WpOLEBase<_ADOUser>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void put_Name(std::u16string_view _rName);
            bool ChangePassword(std::u16string_view _rPwd,std::u16string_view _rNewPwd);
            WpADOGroups get_Groups();
            RightsEnum GetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType);
            bool SetPermissions(
                /* [in] */ const OLEVariant& Name,
                /* [in] */ ObjectTypeEnum ObjectType,
                /* [in] */ ActionEnum Action,
                /* [in] */ RightsEnum Rights);
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
