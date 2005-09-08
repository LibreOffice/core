/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:49:48 $
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

#ifndef SC_SERVOBJ_HXX
#define SC_SERVOBJ_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVT_LISTENER_HXX
#include <svtools/listener.hxx>
#endif
#ifndef _LINKSRC_HXX //autogen
#include <sfx2/linksrc.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

class ScDocShell;
class ScServerObject;

class ScServerObjectSvtListenerForwarder : public SvtListener
{
    ScServerObject* pObj;
    SfxBroadcaster  aBroadcaster;
public:
                    ScServerObjectSvtListenerForwarder( ScServerObject* pObjP);
    virtual         ~ScServerObjectSvtListenerForwarder();
    virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint);
};

class ScServerObject : public ::sfx2::SvLinkSource, public SfxListener
{
private:
    ScServerObjectSvtListenerForwarder  aForwarder;
    ScDocShell*     pDocSh;
    ScRange         aRange;
    String          aItemStr;
    BOOL            bRefreshListener;

    void    Clear();

public:
            ScServerObject( ScDocShell* pShell, const String& rItem );
    virtual ~ScServerObject();

    virtual BOOL GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                             const String & rMimeType,
                             BOOL bSynchron = FALSE );

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );
            void    EndListeningAll();
};

//SO2_DECL_REF( ScServerObject )


#endif
