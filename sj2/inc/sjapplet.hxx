/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sjapplet.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SJ_SJAPPLET_HXX
#define _SJ_SJAPPLET_HXX

#include "com/sun/star/uno/Reference.hxx"
#include <tools/string.hxx>

class INetURLObject;
class Size;
class SjJScriptAppletObject;
class SvCommandList;
class Window;
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

struct SjApplet2_Impl;

enum SjNetAccess
{
    NET_UNRESTRICTED, NET_NONE, NET_HOST
};

class SjApplet2
{
    SjApplet2_Impl * _pImpl;

public:
    static void settingsChanged(void);

    SjApplet2();
    virtual ~SjApplet2();
    void Init(
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > const & context,
        Window * pParentWin, const INetURLObject & rDocBase,
        const SvCommandList & rCmdList);
    void setSizePixel( const Size & );
    void appletRestart();
    void appletReload();
    void appletStart();
    void appletStop();
    void appletClose();

    // Fuer SO3, Wrapper fuer Applet liefern
    SjJScriptAppletObject * GetJScriptApplet();

    virtual void appletResize( const Size & ) = 0;
    virtual void showDocument( const INetURLObject &, const XubString & ) = 0;
    virtual void showStatus( const XubString & ) = 0;
//      virtual SjJSbxObject * getJScriptWindowObj() = 0;
};



#endif // _REF_HXX
