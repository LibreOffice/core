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
 *  For LWP filter architecture prototype
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSECTION_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSECTION_HXX
#include "lwppagelayout.hxx"

/**
 * @brief       COrderedObject class in .lwp file
 *
 */
class LwpDLNFVList;
class LwpOrderedObject : public LwpDLNFVList
{
public:
    LwpOrderedObject(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    void Read() override;
    LwpObjectID& GetPosition(){ return m_Para;}
    LwpObjectID& GetListList(){ return m_ListList;}
private:
    LwpObjectID m_ListList;//lwplistlist object,
    LwpObjectID m_Para;
};

/**
 * @brief       VO_SECTION object in .lwp file
 *
 */
class LwpColor;
class LwpAtomHolder;
class LwpSection : public LwpOrderedObject
{
public:
    LwpSection(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpSection() override;
    inline LwpPageLayout* GetPageLayout();
    void Parse(IXFStream* pOutputStream) override;
    OUString const & GetSectionName(){return m_AtomHolder.str();}

protected:
    void Read() override;
private:
    sal_uInt16 m_Flags;
    LwpObjectID m_PageLayout;
    LwpColor m_Color;
    LwpAtomHolder m_AtomHolder;

};

/**
 * @descr Get page layout pointer
 *
 */
inline LwpPageLayout* LwpSection::GetPageLayout()
{
    return dynamic_cast<LwpPageLayout*>(m_PageLayout.obj().get());
}

class LwpIndexSection : public LwpSection
{
public:
    LwpIndexSection(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual ~LwpIndexSection() override;
    bool IsFormatRunin();
    bool IsFormatSeparator();
protected:
    void Read() override;
private:
    enum{
        SEPARATORS = 0x01,
        RUN_IN = 0X02,
        };
    LwpAtomHolder m_TextMarker;
    LwpAtomHolder m_ParentName;
    LwpAtomHolder m_DivisionName;
    LwpAtomHolder m_SectionName;
    sal_uInt16 m_nForm;
    sal_uInt16 m_nFlags;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
