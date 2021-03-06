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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <sal/types.h>

namespace com::sun::star {
    namespace text { class XTextContent; }
}

class XMLTextListAutoStylePool;

/** information about list and list style for a certain paragraph

    OD 2008-04-24 #refactorlists#
    Complete refactoring of the class and enhancement of the class for lists.
    These changes are considered by method <XMLTextParagraphExport::exportListChange(..)>
*/
class XMLTextNumRuleInfo
{
    // numbering rules instance and its name
    css::uno::Reference < css::container::XIndexReplace > mxNumRules;
    OUString            msNumRulesName;

    // paragraph's list attributes
    OUString            msListId;
    sal_Int16           mnListStartValue;
    sal_Int16           mnListLevel;
    bool                mbIsNumbered;
    bool                mbIsRestart;

    // numbering rules' attributes
    sal_Int16           mnListLevelStartValue;

    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    bool                mbOutlineStyleAsNormalListStyle;

    bool                mbContinueingPreviousSubTree;
    OUString            msListLabelString;

public:

    XMLTextNumRuleInfo();

    inline XMLTextNumRuleInfo& operator=( const XMLTextNumRuleInfo& rInfo );

    void Set( const css::uno::Reference < css::text::XTextContent > & rTextContent,
              const bool bOutlineStyleAsNormalListStyle,
              const XMLTextListAutoStylePool& rListAutoPool,
              const bool bExportTextNumberElement );
    inline void Reset();

    const OUString& GetNumRulesName() const
    {
        return msNumRulesName;
    }
    sal_Int16 GetListLevelStartValue() const
    {
        return mnListLevelStartValue;
    }

    const OUString& GetListId() const
    {
        return msListId;
    }

    sal_Int16 GetLevel() const
    {
        return mnListLevel;
    }

    bool HasStartValue() const
    {
        return mnListStartValue != -1;
    }
    sal_uInt32 GetStartValue() const
    {
        return mnListStartValue;
    }

    bool IsNumbered() const
    {
        return mbIsNumbered;
    }
    bool IsRestart() const
    {
        return mbIsRestart;
    }

    bool BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const;

    bool IsContinueingPreviousSubTree() const
    {
        return mbContinueingPreviousSubTree;
    }
    const OUString& ListLabelString() const
    {
        return msListLabelString;
    }
};

inline XMLTextNumRuleInfo& XMLTextNumRuleInfo::operator=(
        const XMLTextNumRuleInfo& rInfo )
{
    msNumRulesName = rInfo.msNumRulesName;
    mxNumRules = rInfo.mxNumRules;
    msListId = rInfo.msListId;
    mnListStartValue = rInfo.mnListStartValue;
    mnListLevel = rInfo.mnListLevel;
    mbIsNumbered = rInfo.mbIsNumbered;
    mbIsRestart = rInfo.mbIsRestart;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbOutlineStyleAsNormalListStyle = rInfo.mbOutlineStyleAsNormalListStyle;
    mbContinueingPreviousSubTree = rInfo.mbContinueingPreviousSubTree;
    msListLabelString = rInfo.msListLabelString;

    return *this;
}

inline void XMLTextNumRuleInfo::Reset()
{
    mxNumRules = nullptr;
    msNumRulesName.clear();
    msListId.clear();
    mnListStartValue = -1;
    mnListLevel = 0;
    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
    mbIsNumbered = mbIsRestart =
    mbOutlineStyleAsNormalListStyle = false;
    mbContinueingPreviousSubTree = false;
    msListLabelString.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
