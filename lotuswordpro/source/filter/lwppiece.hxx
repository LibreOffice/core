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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPIECE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPIECE_HXX

#include <lwpoverride.hxx>
#include "lwpdlvlist.hxx"
#include "lwpbreaksoverride.hxx"
#include "lwpparaborderoverride.hxx"
#include "lwpnumberingoverride.hxx"
#include "lwpcharborderoverride.hxx"
#include "lwpbackgroundoverride.hxx"
#include "lwptaboverride.hxx"

class LwpVirtualPiece : public LwpDLVList
{
public:
    LwpVirtualPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpDLVList(objHdr, pStrm)
    {
    }

    virtual void Read() override
    {
        LwpDLVList::Read();
        if( m_pOverride )
            m_pOverride->Read(m_pObjStrm.get());
    }

    LwpOverride* GetOverride()
    {
        return m_pOverride.get();
    }
protected:
    virtual ~LwpVirtualPiece() override
    {
    }

    std::unique_ptr<LwpOverride>  m_pOverride;
};

class LwpParaBorderPiece : public LwpVirtualPiece
{
public:
    LwpParaBorderPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpParaBorderOverride());
    }

private:
    virtual ~LwpParaBorderPiece() override {}
};

class LwpBreaksPiece : public LwpVirtualPiece
{
public:
    LwpBreaksPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpBreaksOverride());
    }

private:
    virtual ~LwpBreaksPiece() override {}
};

class LwpNumberingPiece : public LwpVirtualPiece
{
public:
    LwpNumberingPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpNumberingOverride());
    }

private:
    virtual ~LwpNumberingPiece() override {}
};

class LwpTabPiece : public LwpVirtualPiece
{
public:
    LwpTabPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpTabOverride());
    }

private:
    virtual ~LwpTabPiece() override {}
};

class LwpBackgroundPiece : public LwpVirtualPiece
{
public:
    LwpBackgroundPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpBackgroundOverride());
    }

private:
    virtual ~LwpBackgroundPiece() override {}
};

class LwpAlignmentPiece : public LwpVirtualPiece
{
public:
    LwpAlignmentPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpAlignmentOverride());
    }

private:
    virtual ~LwpAlignmentPiece() override {}
};

class LwpIndentPiece : public LwpVirtualPiece
{
public:
    LwpIndentPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpIndentOverride());
    }

private:
    virtual ~LwpIndentPiece() override {}
};

class LwpSpacingPiece : public LwpVirtualPiece
{
public:
    LwpSpacingPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpSpacingOverride());
    }

private:
    virtual ~LwpSpacingPiece() override {}
};

class LwpAmikakePiece : public LwpVirtualPiece
{
public:
    LwpAmikakePiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpAmikakeOverride());
    }

private:
    virtual ~LwpAmikakePiece() override {}
};

class LwpCharacterBorderPiece : public LwpVirtualPiece
{
public:
    LwpCharacterBorderPiece(LwpObjectHeader const & objHdr, LwpSvStream* pStrm):LwpVirtualPiece(objHdr, pStrm)
    {
        m_pOverride.reset(new LwpCharacterBorderOverride());
    }

private:
    virtual ~LwpCharacterBorderPiece() override {}
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
