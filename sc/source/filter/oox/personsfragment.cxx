/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <personsfragment.hxx>

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <document.hxx>
#include <postit.hxx>

namespace oox::xls
{
PersonsFragment::PersonsFragment(const WorkbookHelper& rHelper, const OUString& rFragmentPath)
    : WorkbookFragmentBase(rHelper, rFragmentPath)
{
}

core::ContextHandlerRef PersonsFragment::onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case core::XML_ROOT_CONTEXT:
            if (nElement == XTHREADED_TOKEN(personList))
                return this;
            break;
        case XTHREADED_TOKEN(personList):
            if (nElement == XTHREADED_TOKEN(person))
            {
                ScPersonData aPerson;
                aPerson.maDisplayName = rAttribs.getXString(XML_displayName, {});
                aPerson.maId = rAttribs.getXString(XML_id, {});
                aPerson.maUserId = rAttribs.getXString(XML_userId, {});
                aPerson.maProviderId = rAttribs.getXString(XML_providerId, {});
                if (!aPerson.maId.isEmpty())
                    getScDocument().AddPerson(aPerson);
            }
            break;
    }
    return {};
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
