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


#ifndef _EVENTDLG_HXX
#define _EVENTDLG_HXX

#include <vcl/menubtn.hxx>

#include <vcl/fixed.hxx>
#include <sfx2/evntconf.hxx>
#include "macropg.hxx"

#include <com/sun/star/frame/XFrame.hpp>

class Menu;
class SfxMacroInfoItem;
class SvxMacroItem;

class SvxEventConfigPage : public _SvxMacroTabPage
{
    FixedText                           aSaveInText;
    ListBox                             aSaveInListBox;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xAppEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace >   m_xDocumentEvents;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable >         m_xDocumentModifiable;

    sal_Bool            bAppConfig;

    DECL_LINK(      SelectHdl_Impl, ListBox* );

    // Forbidden and not implemented.
    SvxEventConfigPage (const SvxEventConfigPage &);
    SvxEventConfigPage & operator= (const SvxEventConfigPage &);

public:
                    /// this is only to let callers know that there is a LateInit which *must* be called
                    struct EarlyInit { };
                    SvxEventConfigPage( Window *pParent, const SfxItemSet& rSet, EarlyInit );
                    ~SvxEventConfigPage();

    void            LateInit( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame );

protected:
    virtual sal_Bool    FillItemSet( SfxItemSet& );
    virtual void    Reset( const SfxItemSet& );
    using _SvxMacroTabPage::Reset;

private:
    void    ImplInitDocument();
};


#endif

