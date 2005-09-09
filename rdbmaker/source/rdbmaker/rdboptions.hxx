/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rdboptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:05:54 $
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

#ifndef _RDBMAKER_RDBOPTIONS_HXX_
#define _RDBMAKER_RDBOPTIONS_HXX_

#include    <codemaker/options.hxx>

class RdbOptions : public Options
{
public:
    RdbOptions()
        : Options()
        , m_generateTypeList(sal_False)
        {}

    ~RdbOptions() {}

    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
            throw( IllegalArgument );

    ::rtl::OString  prepareHelp();

    ::rtl::OString  prepareVersion();

    sal_Bool generateTypeList()
        { return m_generateTypeList; }

protected:
    sal_Bool m_generateTypeList;
};

#endif // _RDBMAKER_RDBOPTIONS_HXX_
