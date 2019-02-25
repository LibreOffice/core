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
/*************************************************************************
 * @file
 *  LwpContent, LwpHeadContent: base class for Word Pro content object
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPCONTENT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPCONTENT_HXX

#include <lwpheader.hxx>
#include <lwpobj.hxx>
#include <lwpatomholder.hxx>
#include "lwplayout.hxx"
#include "lwphyperlinkmgr.hxx"
/**
 * @brief
 * Base class of all Lwp content objects
 */
class LwpContent : public LwpDLNFVList
{
public:
    LwpContent(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
private:
    LwpAssociatedLayouts m_LayoutsWithMe;
    sal_uInt16 m_nFlags;
    LwpAtomHolder m_ClassName;
    LwpObjectID m_NextEnumerated;
    LwpObjectID m_PreviousEnumerated;
    /* Flag bits */
    enum
    {
        CF_CHANGED              =   0x0001,
        CF_MODIFIED             =   0x0004,
        CF_LOCAL                =   0x0008,
        CF_INFOOTNOTE           =   0x0010,
        CF_METACHANGED          =   0x0020,
        CF_PRIVATE              =   0x0040,
        CF_DEACTIVATED          =   0x0080,
        CF_VALUECHANGED         =   0x0100,
        CF_DISABLEVALUECHECKING =   0x0200,
        CF_DONTNOTIFYDOCUMENT   =   0x0400,     // for numeric formatting
        CF_ISOVERRIDE           =   0x0800,
        CF_ISBULLET             =   0x1000,
        CF_CONTINUEDON          =   0x2000,
        CF_CONTINUEDFROM        =   0x4000,

        // Bits that should be transferred when we duplicate a content
        CF_COPYMASK             = (CF_MODIFIED | CF_PRIVATE)
    };
protected:
    void Read() override;
public:
    inline LwpAssociatedLayouts& GetLayoutsWithMe();
    rtl::Reference<LwpVirtualLayout> GetLayout(LwpVirtualLayout const * pStartLayout);
    inline bool IsActive();
    virtual bool IsTable();
    inline OUString const & GetClassName();
    inline LwpContent* GetNextEnumerated();
    bool HasNonEmbeddedLayouts();
    bool IsStyleContent();
};

LwpAssociatedLayouts& LwpContent::GetLayoutsWithMe()
{
    return m_LayoutsWithMe;
}

inline bool LwpContent::IsActive()
{
    return !(m_nFlags & CF_DEACTIVATED);
}

inline bool LwpContent::IsTable()
{
    return false;
}

inline OUString const & LwpContent::GetClassName()
{
    return m_ClassName.str();
}

inline LwpContent* LwpContent::GetNextEnumerated()
{
    return dynamic_cast<LwpContent*>(m_NextEnumerated.obj().get());
}
/**
 * @brief
 * Head object of content list
 */
class LwpHeadContent : public LwpContent
{
public:
    LwpHeadContent(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
protected:
    void Read() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
