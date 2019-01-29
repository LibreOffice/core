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

#ifndef INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX
#define INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX

#include <vcl/ctrl.hxx>


namespace svt
{


/** This abstract class provides methods to implement an extension list box.
    This header is needed for the automatic test tool
*/
class IExtensionListBox: public Control
{
public:
    enum { ENTRY_NOTFOUND = -1 };

    IExtensionListBox( vcl::Window* pParent ): Control( pParent, WB_BORDER | WB_TABSTOP ){}

    /** @return  The count of the entries in the list box. */
    virtual sal_Int32 getItemCount() const = 0;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns ENTRY_NOTFOUND */
    virtual sal_Int32 getSelIndex() const = 0;

};


} // namespace svt


#endif // INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
