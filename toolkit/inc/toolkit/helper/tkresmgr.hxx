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



#ifndef _TOOLKIT_HELPER_TKRESMGR_HXX_
#define _TOOLKIT_HELPER_TKRESMGR_HXX_

#include <rtl/ustring.hxx>
#include <vcl/image.hxx>

class SimpleResMgr;
class ResMgr;

#define TK_RES_STRING(id)   TkResMgr::loadString(id)
#define TK_RES_IMAGE(id)    TkResMgr::loadImage(id)

// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

class TkResMgr
{
    static SimpleResMgr* m_pSimpleResMgr;
    static ResMgr* m_pResMgr;

private:
    // no instantiation allowed
    TkResMgr() { }
    ~TkResMgr() { }

    // we'll instantiate one static member of the following class,
    // which in it's dtor ensures that m_pSimpleResMgr will be deleted
    class EnsureDelete
    {
    public:
        EnsureDelete() { }
        ~EnsureDelete();
    };
    friend class EnsureDelete;

protected:
    static void ensureImplExists();

public:
    // loads the string with the specified resource id
    static ::rtl::OUString loadString( sal_uInt16 nResId );

    // loads the image with the specified resource id
    static Image loadImage( sal_uInt16 nResId );
    static Image getImageFromURL( const ::rtl::OUString& i_rImageURL );
};


#endif // _TOOLKIT_HELPER_TKRESMGR_HXX_

