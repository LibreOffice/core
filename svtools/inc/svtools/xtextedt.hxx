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


#ifndef _XTEXTEDT_HXX
#define _XTEXTEDT_HXX

#include "svtools/svtdllapi.h"
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>

namespace com {
namespace sun {
namespace star {
namespace util {
    struct SearchOptions;
}}}}

class SVT_DLLPUBLIC ExtTextEngine : public TextEngine
{
private:
    String              maGroupChars;

public:
                        ExtTextEngine();
                        ~ExtTextEngine();

    const String&       GetGroupChars() const { return maGroupChars; }
    void                SetGroupChars( const String& r ) { maGroupChars = r; }
    TextSelection       MatchGroup( const TextPaM& rCursor ) const;

    sal_Bool                Search( TextSelection& rSel, const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bForward = sal_True );
};

class SVT_DLLPUBLIC ExtTextView : public TextView
{
protected:
    sal_Bool                ImpIndentBlock( sal_Bool bRight );

public:
                        ExtTextView( ExtTextEngine* pEng, Window* pWindow );
                        ~ExtTextView();

    sal_Bool                MatchGroup();

    sal_Bool                Search( const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bForward );
    sal_uInt16              Replace( const ::com::sun::star::util::SearchOptions& rSearchOptions, sal_Bool bAll, sal_Bool bForward );

    sal_Bool                IndentBlock();
    sal_Bool                UnindentBlock();
};

#endif // _XTEXTEDT_HXX
