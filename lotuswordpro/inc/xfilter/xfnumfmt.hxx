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
 * Number format. a),A),i),1),(1)
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFNUMFMT_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFNUMFMT_HXX

#include <xfilter/ixfstream.hxx>
#include <xfilter/ixfattrlist.hxx>

/**
 * @brief
 * Bullet number format.
 */
class XFNumFmt
{
public:
    XFNumFmt()
    {
        m_nStartValue = 0;
    }

public:
    /**********************************************************************
     * @descr:      set the prefix of the number format,that the '(' of output
                    (1 item1
                    (2 item2
     * @param:      prefix
     *********************************************************************/
    void    SetPrefix(const OUString& prefix)
    {
        m_strPrefix = prefix;
    }

    /**********************************************************************
     * @descr:      set the suffix of the number format,that the ')' of output
                    1) item1
                    2) item2
     * @param:      prefix
     *********************************************************************/
    void    SetSuffix(const OUString& suffix)
    {
        m_strSuffix = suffix;
    }

    /**********************************************************************
     * @descr:      set the display value. ie. if you set format to '1',then
                    the output would be:
                        1 item1
                        2 item2
                    or if you set the format to 'a', then the output would be:
                        a item1
                        b item2
                    some special, if you set the format to a sequence,ie. "一,二,三,..."
                    the output would be:
                        一 item1
                        二 item2
                        三 item3

                    Of course, for unordered-list, you can also set the format to
                    be a bullet char,ie:
                        '','','','','',
     * @param:      prefix
     *********************************************************************/
    void    SetFormat(const OUString& format)
    {
        m_strFormat = format;
    }

    void    SetStartValue(sal_Int16 start)
    {
        m_nStartValue = start;
    }

    void ToXml(IXFStream *pStrm)
    {
        IXFAttrList *pAttrList = pStrm->GetAttrList();

        if( !m_strPrefix.isEmpty() )
            pAttrList->AddAttribute( u"style:num-prefix"_ustr, m_strPrefix );
        if( !m_strSuffix.isEmpty() )
            pAttrList->AddAttribute( u"style:num-suffix"_ustr, m_strSuffix );
        pAttrList->AddAttribute( u"style:num-format"_ustr, m_strFormat );
        if( m_nStartValue != 0 )
            pAttrList->AddAttribute( u"text:start-value"_ustr, OUString::number(static_cast<sal_Int32>(m_nStartValue)) );
    }
private:
    OUString   m_strPrefix;
    OUString   m_strSuffix;
    OUString   m_strFormat;
    sal_Int16   m_nStartValue;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
