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

#ifndef INCLUDED_SD_INC_TEXTAPI_HXX
#define INCLUDED_SD_INC_TEXTAPI_HXX

#include <editeng/unoedsrc.hxx>
#include <editeng/unotext.hxx>
#include <editeng/eeitem.hxx>
#include <rtl/ref.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoipset.hxx>

class SdDrawDocument;

namespace sd {

class TextAPIEditSource;

class TextApiObject : public SvxUnoText
{
public:
    static rtl::Reference< TextApiObject > create( SdDrawDocument* pDoc );

    virtual             ~TextApiObject() throw();

    void SAL_CALL dispose() throw(css::uno::RuntimeException);

    OutlinerParaObject* CreateText();
    void                SetText( OutlinerParaObject& rText );
    OUString            GetText();

    static TextApiObject* getImplementation( const css::uno::Reference< css::text::XText >& );

private:
    TextAPIEditSource*  mpSource;
    TextApiObject( TextAPIEditSource* pEditSource );
};

} // namespace sd

#endif // INCLUDED_SD_INC_TEXTAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
