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


#ifndef _TEXTMARKUPHELPER_HXX_
#define _TEXTMARKUPHELPER_HXX_

#include <sal/types.h>
#include "com/sun/star/lang/IndexOutOfBoundsException.hdl"
#include "com/sun/star/lang/IllegalArgumentException.hdl"
#include "com/sun/star/uno/RuntimeException.hdl"
#include "com/sun/star/uno/Sequence.h"

namespace com { namespace sun { namespace star { namespace accessibility {
struct TextSegment;
} } } }

class SwAccessiblePortionData;
class SwTxtNode;
// --> OD 2010-02-19 #i108125#
class SwWrongList;
// <--

class SwTextMarkupHelper
{
    public:
        SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                            const SwTxtNode& rTxtNode );
        // --> OD 2010-02-19 #i108125#
        SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                            const SwWrongList& rTextMarkupList );
        // <--
        ~SwTextMarkupHelper() {}

        sal_Int32 getTextMarkupCount( const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

        ::com::sun::star::accessibility::TextSegment getTextMarkup(
                                            const sal_Int32 nTextMarkupIndex,
                                            const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                       ::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Sequence< ::com::sun::star::accessibility::TextSegment >
                getTextMarkupAtIndex( const sal_Int32 nCharIndex,
                                      const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                       ::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

    private:
        SwTextMarkupHelper( const SwTextMarkupHelper& );
        SwTextMarkupHelper& operator=( const SwTextMarkupHelper& );

        const SwAccessiblePortionData& mrPortionData;

        // --> OD 2010-02-19 #i108125#
        const SwTxtNode* mpTxtNode;
        const SwWrongList* mpTextMarkupList;
        // <--
};
#endif

