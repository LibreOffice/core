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



#if !defined INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
#define INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX

#include "svtools/svtdllapi.h"
#include <toolkit/awt/vclxwindow.hxx>

#include <memory>

class TextEngine;
class TextView;

namespace svt
{
    class AccessibleFactoryAccess ;

class TextWindowPeer: public ::VCLXWindow
{
public:
    SVT_DLLPUBLIC TextWindowPeer(::TextView & rView, bool bCompoundControlChild = false);

    virtual ~TextWindowPeer();

private:
    // VCLXWindow inherits funny copy constructor and assignment operator from
    // ::cppu::OWeakObject, so override them here:

    TextWindowPeer(TextWindowPeer &); // not implemented

    void operator =(TextWindowPeer); // not implemented

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext >
    CreateAccessibleContext();

    ::TextEngine & m_rEngine;
    ::TextView & m_rView;
    bool m_bCompoundControlChild;

    ::std::auto_ptr< ::svt::AccessibleFactoryAccess  >  m_pFactoryAccess;
};

}

#endif // INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
