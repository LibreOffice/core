/*************************************************************************
 *
 *  $RCSfile: streamsection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 13:33:25 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace comphelper
{

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataInputStream >& _rxInput)
    :m_xInStream(_rxInput)
    ,m_xMarkStream(_rxInput, ::com::sun::star::uno::UNO_QUERY)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xInStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xInStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = _rxInput->readLong();
        m_nBlockStart = m_xMarkStream->createMark();
    }
}

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataOutputStream >& _rxOutput, sal_Int32 _nPresumedLength)
    :m_xOutStream(_rxOutput)
    ,m_xMarkStream(_rxOutput, ::com::sun::star::uno::UNO_QUERY)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xOutStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockStart = m_xMarkStream->createMark();
        // a placeholder where we will write the overall length (within the destructor)
        if (_nPresumedLength > 0)
            m_nBlockLen = _nPresumedLength + sizeof(m_nBlockLen);
            // as the caller did not consider - of course - the placeholder we are going to write
        else
            m_nBlockLen = 0;
        m_xOutStream->writeLong(m_nBlockLen);
    }
}

//-------------------------------------------------------------------------
OStreamSection::~OStreamSection()
{
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        // handling routing
        if (m_xInStream.is() &&  m_xMarkStream.is())
        {   // we're working on an input stream
            m_xMarkStream->jumpToMark(m_nBlockStart);
            m_xInStream->skipBytes(m_nBlockLen);
            m_xMarkStream->deleteMark(m_nBlockStart);
        }
        else if (m_xOutStream.is() && m_xMarkStream.is())
        {
            sal_Int32 nRealBlockLength = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
            if (m_nBlockLen && (m_nBlockLen == nRealBlockLength))
                // nothing to do : the estimation the caller gave us (in the ctor) was correct
                m_xMarkStream->deleteMark(m_nBlockStart);
            else
            {   // the estimation was wrong (or we didn't get one)
                m_nBlockLen = nRealBlockLength;
                m_xMarkStream->jumpToMark(m_nBlockStart);
                m_xOutStream->writeLong(m_nBlockLen);
                m_xMarkStream->jumpToFurthest();
                m_xMarkStream->deleteMark(m_nBlockStart);
            }
        }
    }
    catch(...)
    {
    }
}

}   // namespace comphelper


