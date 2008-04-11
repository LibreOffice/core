/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include    <codemaker/options.hxx>

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

sal_Bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    if (m_options.count(option) > 0)
    {
        return m_options[option];
    } else
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }
}

const OptionMap& Options::getOptions()
{
    return m_options;
}

const OString Options::getInputFile(sal_uInt16 index)
    throw( IllegalArgument )
{
    if (index < m_inputFiles.size())
    {
        return m_inputFiles[index];
    } else
    {
        throw IllegalArgument("index is out of bound.");
    }
}

const StringVector& Options::getInputFiles()
{
    return m_inputFiles;
}

