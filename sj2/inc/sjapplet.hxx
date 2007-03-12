/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sjapplet.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-12 10:46:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SJ_SJAPPLET_HXX
#define _SJ_SJAPPLET_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
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
