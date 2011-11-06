/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#define _XMLOFF_XMLTEXTNUMRULEINFO_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }
#include <sal/types.h>

class XMLTextListAutoStylePool;

/** information about list and list style for a certain paragraph

    OD 2008-04-24 #refactorlists#
    Complete refactoring of the class and enhancement of the class for lists.
    These changes are considered by method <XMLTextParagraphExport::exportListChange(..)>
*/
class XMLTextNumRuleInfo
{
    const ::rtl::OUString msNumberingRules;
    const ::rtl::OUString msNumberingLevel;
    const ::rtl::OUString msNumberingStartValue;
    const ::rtl::OUString msParaIsNumberingRestart;
    const ::rtl::OUString msNumberingIsNumber;
    const ::rtl::OUString msNumberingIsOutline;
    const ::rtl::OUString msPropNameListId;
    const ::rtl::OUString msPropNameStartWith;
    // --> OD 2008-11-26 #158694#
    const ::rtl::OUString msContinueingPreviousSubTree;
    const ::rtl::OUString msListLabelStringProp;
    // <--

    // numbering rules instance and its name
    ::com::sun::star::uno::Reference <
                        ::com::sun::star::container::XIndexReplace > mxNumRules;
    ::rtl::OUString     msNumRulesName;

    // paragraph's list attributes
    ::rtl::OUString     msListId;
    sal_Int16           mnListStartValue;
    sal_Int16           mnListLevel;
    sal_Bool            mbIsNumbered;
    sal_Bool            mbIsRestart;

    // numbering rules' attributes
    // --> OD 2008-05-07 #refactorlists#
    sal_Int16           mnListLevelStartValue;
    // <--

    // --> OD 2006-09-27 #i69627#
    sal_Bool mbOutlineStyleAsNormalListStyle;
    // <--

    // --> OD 2008-11-26 #158694#
    sal_Bool mbContinueingPreviousSubTree;
    ::rtl::OUString msListLabelString;
    // <--
public:

    XMLTextNumRuleInfo();

    inline XMLTextNumRuleInfo& operator=( const XMLTextNumRuleInfo& rInfo );

    // --> OD 2008-11-26 #158694#
    void Set( const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & rTextContnt,
              const sal_Bool bOutlineStyleAsNormalListStyle,
              const XMLTextListAutoStylePool& rListAutoPool,
              const sal_Bool bExportTextNumberElement );
    // <--
    inline void Reset();

    inline const ::rtl::OUString& GetNumRulesName() const
    {
        return msNumRulesName;
    }
    inline const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
    {
        return mxNumRules;
    }
    inline sal_Int16 GetListLevelStartValue() const
    {
        return mnListLevelStartValue;
    }

    inline const ::rtl::OUString& GetListId() const
    {
        return msListId;
    }

    inline sal_Int16 GetLevel() const
    {
        return mnListLevel;
    }

    inline sal_Bool HasStartValue() const
    {
        return mnListStartValue != -1;
    }
    inline sal_uInt32 GetStartValue() const
    {
        return mnListStartValue;
    }

    inline sal_Bool IsNumbered() const
    {
        return mbIsNumbered;
    }
    inline sal_Bool IsRestart() const
    {
        return mbIsRestart;
    }

    sal_Bool BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const;

    inline sal_Bool HasSameNumRules( const XMLTextNumRuleInfo& rCmp ) const
    {
        return rCmp.msNumRulesName == msNumRulesName;
    }

    // --> OD 2008-11-26 #158694#
    inline sal_Bool IsContinueingPreviousSubTree() const
    {
        return mbContinueingPreviousSubTree;
    }
    inline const ::rtl::OUString& ListLabelString() const
    {
        return msListLabelString;
    }
    // <--
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
    // --> OD 2006-09-27 #i69627#
    mbOutlineStyleAsNormalListStyle = rInfo.mbOutlineStyleAsNormalListStyle;
    // <--
    // --> OD 2008-11-26 #158694#
    mbContinueingPreviousSubTree = rInfo.mbContinueingPreviousSubTree;
    msListLabelString = rInfo.msListLabelString;
    // <--

    return *this;
}

inline void XMLTextNumRuleInfo::Reset()
{
    mxNumRules = 0;
    msNumRulesName = ::rtl::OUString();
    msListId = ::rtl::OUString();
    mnListStartValue = -1;
    mnListLevel = 0;
    // --> OD 2006-09-27 #i69627#
    mbIsNumbered = mbIsRestart =
    mbOutlineStyleAsNormalListStyle = sal_False;
    // <--
    // --> OD 2008-11-26 #158694#
    mbContinueingPreviousSubTree = sal_False;
    msListLabelString = ::rtl::OUString();
    // <--
}
#endif  //  _XMLOFF_XMLTEXTNUMRULEINFO_HXX
