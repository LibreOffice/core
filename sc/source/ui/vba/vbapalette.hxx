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



#ifndef SC_VBAPALETTE_HXX
#define SC_VBAPALETTE_HXX

#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace frame { class XModel; }
} } }

class SfxObjectShell;

class ScVbaPalette
{
private:
    SfxObjectShell* m_pShell;
public:
    ScVbaPalette( SfxObjectShell* pShell = 0 ) : m_pShell( pShell ) {}
    ScVbaPalette( const css::uno::Reference< css::frame::XModel >& rxModel );
    // if no palette available e.g. because the document doesn't have a
    // palette defined then a default palette will be returned.
    css::uno::Reference< css::container::XIndexAccess > getPalette() const;
    static css::uno::Reference< css::container::XIndexAccess > getDefaultPalette();
};

#endif //SC_VBAPALETTE_HXX

