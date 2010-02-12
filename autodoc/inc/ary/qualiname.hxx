/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef ARY_QUALINAME_HXX
#define ARY_QUALINAME_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/tpl/tpltools.hxx>


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




}   // namespace ary
#endif
