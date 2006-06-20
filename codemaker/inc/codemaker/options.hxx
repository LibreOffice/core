/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:22:39 $
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

#ifndef INCLUDED_CODEMAKER_OPTIONS_HXX
#define INCLUDED_CODEMAKER_OPTIONS_HXX

#include <hash_map>

#ifndef INCLUDED_CODEMAKER_GLOBAL_HXX
#include <codemaker/global.hxx>
#endif

#if defined( _MSC_VER ) && ( _MSC_VER < 1200 )
typedef ::std::__hash_map__
<
    ::rtl::OString,
    ::rtl::OString,
    HashString,
    EqualString,
    NewAlloc
> OptionMap;
#else
typedef ::std::hash_map
<
    ::rtl::OString,
    ::rtl::OString,
    HashString,
    EqualString
> OptionMap;
#endif

class IllegalArgument
{
public:
    IllegalArgument(const ::rtl::OString& msg)
        : m_message(msg) {}

    ::rtl::OString  m_message;
};

class Options
{
public:
    Options();
    virtual ~Options();

    virtual sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False)
        throw( IllegalArgument ) = 0;

    virtual ::rtl::OString  prepareHelp() = 0;

    const ::rtl::OString&   getProgramName() const;
    sal_Bool                isValid(const ::rtl::OString& option);
    const ::rtl::OString    getOption(const ::rtl::OString& option)
        throw( IllegalArgument );
    const OptionMap&        getOptions();

    const ::rtl::OString    getInputFile(sal_uInt16 index)
        throw( IllegalArgument );

    const StringVector& getInputFiles();

    ::rtl::OString getExtraInputFile(sal_uInt16 index) const throw( IllegalArgument );
    inline sal_uInt16 getNumberOfExtraInputFiles() const
        { return (sal_uInt16)m_extra_input_files.size(); }
    inline const StringVector& getExtraInputFiles() const
        { return m_extra_input_files; }
protected:
    ::rtl::OString  m_program;
    StringVector    m_inputFiles;
    StringVector    m_extra_input_files;
    OptionMap       m_options;
};

#endif // INCLUDED_CODEMAKER_OPTIONS_HXX

