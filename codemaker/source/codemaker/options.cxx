/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: options.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:10:16 $
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

#ifndef _CODEMAKER_OPTIONS_HXX_
#include    <codemaker/options.hxx>
#endif

using namespace rtl;

Options::Options()
{
}

Options::~Options()
{

}

const OString& Options::getProgramName() const
{
    return m_program;
}

sal_uInt16 Options::getNumberOfOptions() const
{
    return m_options.size();
}

sal_Bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    const OString ret;

    if (m_options.count(option) > 0)
    {
        return m_options[option];
    } else
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }

    return ret;
}

const OptionMap& Options::getOptions()
{
    return m_options;
}

sal_uInt16 Options::getNumberOfInputFiles() const
{
    return m_inputFiles.size();
}

const OString Options::getInputFile(sal_uInt16 index)
    throw( IllegalArgument )
{
    const OString ret;

    if (index < m_inputFiles.size())
    {
        return m_inputFiles[index];
    } else
    {
        throw IllegalArgument("index is out of bound.");
    }

    return ret;
}

const StringVector& Options::getInputFiles()
{
    return m_inputFiles;
}

OString Options::getExtraInputFile(sal_uInt16 index) const
    throw( IllegalArgument )
{
    if (index < m_extra_input_files.size())
    {
        return m_extra_input_files[index];
    } else
    {
        throw IllegalArgument("index is out of bound.");
    }

    return OString();
}

