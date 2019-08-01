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
#ifndef INCLUDED_SFX2_OPENGRF_HXX
#define INCLUDED_SFX2_OPENGRF_HXX

#include <memory>
#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/errcode.hxx>

namespace com::sun::star::ui::dialogs { class XFilePickerControlAccess; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace weld { class Window; }

class Graphic;
struct  SvxOpenGrf_Impl;

class SFX2_DLLPUBLIC SvxOpenGraphicDialog
{
public:
    SvxOpenGraphicDialog(const OUString& rTitle, weld::Window* pPreferredParent);
    SvxOpenGraphicDialog(const OUString& rTitle, weld::Window* pPreferredParent,
                         sal_Int16 nDialogType);
    ~SvxOpenGraphicDialog();

    ErrCode                 Execute();

    void                    SetPath( const OUString& rPath, bool bLinkState );
    OUString                GetPath() const;

    ErrCode                 GetGraphic(Graphic&) const;

    void                    EnableLink(bool);
    void                    AsLink(bool);
    bool                    IsAsLink() const;

    //what the dialog thought the format was
    OUString                GetCurrentFilter() const;
    void                    SetCurrentFilter(const OUString&);

    //what was subsequently found to be the format
    OUString const &        GetDetectedFilter() const;
    void                    SetDetectedFilter(const OUString&);

    css::uno::Reference<css::ui::dialogs::XFilePickerControlAccess> const & GetFilePickerControlAccess() const;

private:
    SvxOpenGraphicDialog    (const SvxOpenGraphicDialog&) = delete;
    SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & ) = delete;

    const std::unique_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
