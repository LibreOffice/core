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


#ifndef _RUBYLIST_HXX
#define _RUBYLIST_HXX

#include <svl/svarray.hxx>
#include <swtypes.hxx>
#include <fmtruby.hxx>

class SwRubyListEntry
{
    String sText;
    SwFmtRuby aRubyAttr;
public:
    SwRubyListEntry() : aRubyAttr( aEmptyStr ) {}
    ~SwRubyListEntry();

    const String& GetText() const               { return sText; }
    void SetText( const String& rStr )          { sText = rStr; }

    const SwFmtRuby& GetRubyAttr() const        { return aRubyAttr; }
          SwFmtRuby& GetRubyAttr()              { return aRubyAttr; }
    void SetRubyAttr( const SwFmtRuby& rAttr )  { aRubyAttr = rAttr; }
};

typedef SwRubyListEntry* SwRubyListEntryPtr;
SV_DECL_PTRARR_DEL( SwRubyList, SwRubyListEntryPtr, 0, 32 )


#endif  //_RUBYLIST_HXX
