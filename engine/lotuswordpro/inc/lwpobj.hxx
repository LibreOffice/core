/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  Base class of all Lwp VO objects
 */

#ifndef INCLUDED_LOTUSWORDPRO_INC_LWPOBJ_HXX
#define INCLUDED_LOTUSWORDPRO_INC_LWPOBJ_HXX

#include <memory>
#include <sal/config.h>

#include <stdexcept>

#include <salhelper/simplereferenceobject.hxx>

#include <config_lgpl.h>

#include "lwpobjhdr.hxx"
#include "lwpobjstrm.hxx"
#include "xfilter/ixfstream.hxx"
#include "xfilter/xfcontentcontainer.hxx"

class LwpFoundry;
/**
 * @brief   Base class of all Lwp VO objects
*/
class LwpObject : public salhelper::SimpleReferenceObject
{
public:
    LwpObject(LwpObjectHeader objHdr, LwpSvStream* pStrm);

protected:
    virtual ~LwpObject() override;
    LwpObjectHeader m_ObjHdr;
    std::unique_ptr<LwpObjectStream> m_pObjStrm;
    LwpFoundry* m_pFoundry;
    LwpSvStream* m_pStrm;
    bool m_bRegisteringStyle;
    bool m_bParsingStyle;
    bool m_bConvertingContent;

protected:
    virtual void Read();
    virtual void RegisterStyle();
    virtual void Parse(IXFStream* pOutputStream);
    virtual void XFConvert(XFContentContainer* pCont);

public:
    void QuickRead();
    //calls RegisterStyle but bails if DoRegisterStyle is called
    //on the same object recursively
    void DoRegisterStyle()
    {
        if (m_bRegisteringStyle)
            throw std::runtime_error("recursion in styles");
        m_bRegisteringStyle = true;
        RegisterStyle();
        m_bRegisteringStyle = false;
    }
    //calls Parse but bails if DoParse is called
    //on the same object recursively
    void DoParse(IXFStream* pOutputStream)
    {
        if (m_bParsingStyle)
            throw std::runtime_error("recursion in parsing");
        m_bParsingStyle = true;
        Parse(pOutputStream);
        m_bParsingStyle = false;
    }
    //calls XFConvert but bails if DoXFConvert is called
    //on the same object recursively
    void DoXFConvert(XFContentContainer* pCont)
    {
        if (m_bConvertingContent)
            throw std::runtime_error("recursion in parsing");
        m_bConvertingContent = true;
        XFConvert(pCont);
        m_bConvertingContent = false;
    }

    LwpFoundry* GetFoundry() { return m_pFoundry; }
    void SetFoundry(LwpFoundry* pFoundry) { m_pFoundry = pFoundry; }
    LwpObjectID& GetObjectID() { return m_ObjHdr.GetID(); }
    sal_uInt32 GetTag() const { return m_ObjHdr.GetTag(); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
