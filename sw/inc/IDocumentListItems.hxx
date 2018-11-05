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

#ifndef INCLUDED_SW_INC_IDOCUMENTLISTITEMS_HXX
#define INCLUDED_SW_INC_IDOCUMENTLISTITEMS_HXX

#include <vector>
#include <rtl/ustring.hxx>

class SwRootFrame;
class SwNodeNum;

/** Provides numbered items of a document.
*/
class IDocumentListItems
{
public:
    typedef std::vector< const SwNodeNum* > tSortedNodeNumList;

    virtual void addListItem( const SwNodeNum& rNodeNum ) = 0;
    virtual void removeListItem( const SwNodeNum& rNodeNum ) = 0;

    virtual OUString getListItemText(const SwNodeNum& rNodeNum,
                                     SwRootFrame const& rLayout) const = 0;

    virtual bool isNumberedInLayout(SwNodeNum const& rNodeNum,
            SwRootFrame const& rLayout) const = 0;

    /** get vector of all list items, which are numbered
    */
    virtual void getNumItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const = 0;

protected:
    virtual ~IDocumentListItems() {};
};

 #endif // INCLUDED_SW_INC_IDOCUMENTLISTITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
