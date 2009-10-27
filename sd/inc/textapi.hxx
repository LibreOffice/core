/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textapi.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SD_TEXTAPI_HXX_
#define _SD_TEXTAPI_HXX_

#include <svx/unoedsrc.hxx>
#include <svx/unotext.hxx>
#include <svx/eeitem.hxx>
#include <rtl/ref.hxx>

class SdDrawDocument;

namespace sd {

class TextAPIEditSource;

class TextApiObject : public SvxUnoText
{
public:
    static rtl::Reference< TextApiObject > create( SdDrawDocument* pDoc );

    virtual             ~TextApiObject() throw();

    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    OutlinerParaObject* CreateText();
    void                SetString( const String& rText );
    void                SetText( OutlinerParaObject& rText );
    String              GetText();

    static TextApiObject* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& );

private:
    TextAPIEditSource*  mpSource;
    TextApiObject( TextAPIEditSource* pEditSource );
};

} // namespace sd


#endif // _SD_TEXTAPI_HXX_
