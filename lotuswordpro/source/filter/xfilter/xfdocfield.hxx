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
 * doc field.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-05-27  create this file.
 ************************************************************************/
#ifndef		_XFDOCFIELD_HXX
#define		_XFDOCFIELD_HXX

#include "xfcontent.hxx"
#include "xfpagecount.hxx"

class XFFileName : public XFContent
{
public:
    void SetType(rtl::OUString sType);
    virtual void	ToXml(IXFStream *pStrm);
private:
    rtl::OUString m_strType;
};
class XFWordCount : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFWordCountStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFWordCountEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFPageCountStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFPageCountEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFCharCount : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFCharCountStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFCharCountEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFDescription : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFDescriptionStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFDescriptionEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFKeywords : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFTotalEditTime : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFTotalEditTimeEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFTotalEditTimeStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFCreateTime : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFCreateTimeStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFCreateTimeEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFLastEditTime : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};
class XFLastEditTimeStart : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFLastEditTimeEnd : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};

class XFInitialCreator : public XFContent
{
public:
    virtual void	ToXml(IXFStream *pStrm);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
