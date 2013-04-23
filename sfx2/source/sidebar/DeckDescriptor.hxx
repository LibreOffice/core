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
#ifndef SFX_SIDEBAR_DECK_DESCRIPTOR_HXX
#define SFX_SIDEBAR_DECK_DESCRIPTOR_HXX

#include "sfx2/sidebar/EnumContext.hxx"
#include "ContextList.hxx"
#include <boost/shared_ptr.hpp>


namespace sfx2 { namespace sidebar {

class DeckDescriptor
{
public:
    ::rtl::OUString msTitle;
    ::rtl::OUString msId;
    ::rtl::OUString msIconURL;
    ::rtl::OUString msHighContrastIconURL;
    ::rtl::OUString msTitleBarIconURL;
    ::rtl::OUString msHighContrastTitleBarIconURL;
    ::rtl::OUString msHelpURL;
    ::rtl::OUString msHelpText;
    ContextList maContextList;
    bool mbIsEnabled;
    sal_Int32 mnOrderIndex;

    DeckDescriptor (void);
    DeckDescriptor (const DeckDescriptor& rOther);
    ~DeckDescriptor (void);
};
typedef ::boost::shared_ptr<DeckDescriptor> SharedDeckDescriptor;

} } // end of namespace sfx2::sidebar

#endif
