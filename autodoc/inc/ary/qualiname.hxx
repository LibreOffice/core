/*************************************************************************
 *
 *  $RCSfile: qualiname.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:10:37 $
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

#ifndef ARY_QUALINAME_HXX
#define ARY_QUALINAME_HXX

//  VERSION:            Autodoc 2.2


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/template/tpltools.hxx>


namespace ary
{

class QualifiedName
{
  public:
    typedef StringVector::const_iterator  namespace_iterator;

                        QualifiedName(
                            uintt               i_nSize = 0);

    /// @see AssignText()
                        QualifiedName(
                            const char *        i_sText,
                            const char *        i_sSeparator );
                        ~QualifiedName();

    QualifiedName &     operator+=(
                            const String &      i_sNamespaceName )
                                                { if (i_sNamespaceName.length() > 0)
                                                        aNamespace.push_back(i_sNamespaceName);
                                                  return *this; }
    /// @precond i_nIndex < NamespaceDepth().
    String &            operator[](
                            uintt               i_nIndex )
                                                { csv_assert(i_nIndex < aNamespace.size());
                                                  return aNamespace[i_nIndex]; }
    void                Init(
                            bool                i_bAbsolute )
                                                { Empty(); bIsAbsolute = i_bAbsolute; }
    /** Reads a qualified name from a string.
        If the last two charcters are "()", the inquiry IsFunction() will return
        true.
    */
    void                AssignText(
                            const char *        i_sText,
                            const char *        i_sSeparator );
    void                SetLocalName(
                            const String &      i_sLocalName )
                                                { sLocalName = i_sLocalName; }
    void                Empty()                 { csv::erase_container(aNamespace); sLocalName.clear(); bIsAbsolute = false; }

    const String &      LocalName() const       { return sLocalName; }
    namespace_iterator  first_namespace() const { return aNamespace.begin(); }
    namespace_iterator  end_namespace() const   { return aNamespace.end(); }
    uintt               NamespaceDepth() const  { return aNamespace.size(); }

    bool                IsAbsolute() const      { return bIsAbsolute; }
    bool                IsQualified() const     { return aNamespace.size() > 0; }
    bool                IsFunction() const      { return bIsFunction; }

  private:
    // DATA
    StringVector        aNamespace;
    String              sLocalName;
    bool                bIsAbsolute;            /// true := beginning with "::".
    bool                bIsFunction;            /// true := ending with "()"
};

// IMPLEMENTATION


}   // namespace ary



#endif

