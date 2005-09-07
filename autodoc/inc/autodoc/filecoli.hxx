/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filecoli.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:26:40 $
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

#ifndef ADC_PARSER_FILECOLI_HXX
#define ADC_PARSER_FILECOLI_HXX


namespace autodoc
{

class FileCollector_Ifc
{
  public:
    // TYPES
    typedef StringVector::const_iterator const_iterator;

    enum E_SearchMode
    {
        flat,
        recursive
    };

    // LIFECYCLE
    virtual             ~FileCollector_Ifc() {}

    // OPERATIONS
    virtual uintt       AddFilesFrom(
                            const char *        i_sRootDir,
                            const char *        i_sFilter,
                            E_SearchMode        i_eSearchMode ) = 0;
    virtual uintt       AddFile(
                            const char *        i_sFilePath ) = 0;
    virtual void        EraseAll() = 0;

    // INQUIRY
    virtual const_iterator
                        Begin() const = 0;
    virtual const_iterator
                        End() const = 0;
    virtual uintt       Size() const = 0;
};


} // namespace autodoc


#endif

