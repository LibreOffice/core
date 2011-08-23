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
 *  For LWP filter architecture prototype - notes
 */
/*************************************************************************
 * Change History
 May 2005		Created
 ************************************************************************/
#ifndef _LWPNOTES_HXX_
#define _LWPNOTES_HXX_

#include "lwpfrib.hxx"
#include "lwpframelayout.hxx"

/**
 * @brief		TAG_NOTE_FRIB object
 *
 */
class LwpFribNote: public LwpFrib
{
public:
    LwpFribNote(LwpPara* pPara );
    ~LwpFribNote(){};
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len);
    void RegisterStyle();
    void XFConvert(XFContentContainer* pCont);
private:
    LwpObjectID m_Layout;
};

/**
 * @brief		VO_NOTELAYOUT object
 *
 */
class LwpNoteLayout: public LwpFrameLayout
{
public:
    LwpNoteLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpNoteLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_NOTE_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);
    sal_uInt32 GetTime(){ return m_nTime;};
    OUString GetAuthor();

protected:
    void Read();
    LwpVirtualLayout* GetTextLayout();

private:
    sal_uInt32 m_nTime;
    LwpAtomHolder m_UserName;
};


/**
 * @brief		VO_NOTEHEADERLAYOUT object
 *
 */
class LwpNoteHeaderLayout: public LwpFrameLayout
{
public:
    LwpNoteHeaderLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpNoteHeaderLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_NOTEHEADER_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);

protected:
    void Read();
};

/**
 * @brief		VO_NOTETEXTLAYOUT object
 *
 */
class LwpNoteTextLayout: public LwpFrameLayout
{
public:
    LwpNoteTextLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpNoteTextLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_NOTETEXT_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);

protected:
    void Read();
};

/**
 * @brief		VO_VPLAYOUT object
 *
 */
class LwpViewportLayout: public LwpPlacableLayout
{
public:
    LwpViewportLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpViewportLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_VIEWPORT_LAYOUT;}
    virtual void RegisterStyle();
    virtual void XFConvert(XFContentContainer* pCont);

protected:
    void Read();
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
