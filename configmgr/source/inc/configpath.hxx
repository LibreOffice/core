/*************************************************************************
 *
 *  $RCSfile: configpath.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: lla $ $Date: 2001-03-27 07:54:08 $
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

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#define CONFIGMGR_CONFIGPATH_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"
#include <vector>

#include "attributes.hxx"

namespace configmgr
{
    namespace configuration
    {
    //------------------------------------------------------------------------
        /// A tag struct for disabling validity checking on arguments
        namespace argument { struct NoValidate {}; }

        class Name;
        class Path;
        class AbsolutePath;
        class RelativePath;
    //------------------------------------------------------------------------

        /// represents a name for a node in the configuration
        class Name
        {
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// construct an empty Name
            Name() : m_sRep() {}

            /// construct a Name from a String without checking whether it is a valid node-name
            explicit Name(OUString const& aString, NoValidate);

            /// check whether this is an empty Name
            bool isEmpty() const { return m_sRep.getLength() == 0; }

            /// get a string representation of this Name
            const OUString& toString() const { return m_sRep; }

        public:
        // comparison operators
            // equality (== is primary)
            friend bool operator==(Name const& lhs, Name const& rhs)
            { return !!(lhs.m_sRep == rhs.m_sRep); }

        // comparison operators
            // ordering (< is primary)
            friend bool operator< (Name const& lhs, Name const& rhs)
            { return !!(lhs.m_sRep < rhs.m_sRep); }

        // hashing support
            size_t hashCode() const { return m_sRep.hashCode(); }
        private:
            OUString m_sRep;
        };
        //--------------------------------------------------------------------

        /** make a <type>Name</type> out of <var>sName</var>
            without full Validation.
        */
        Name makeName(OUString const& sName);

        /** make a <type>Name</type> out of <var>sName</var>,
            validating that it can be used for a config Node (excluding set elements)
            or template name.
            @throws InvalidName
                if the name is not valid for that purpose
        */
        Name validateNodeName(OUString const& sName);

        /** make a <type>Name</type> out of <var>sName</var>
            validating that it can be used for a config set element
            or template name.
            @throws InvalidName
                if the name is not valid for that purpose
        */
        Name validateElementName(OUString const& sName);
    //------------------------------------------------------------------------

        //--------------------------------------------------------------------
        /// holds information about a node in the schema
        struct NodeInfo
        {
            Name        aName;
            configuration::Attributes aAttributes;
        };
    //-------------------------------------------------------------------------

    //------------------------------------------------------------------------
        /// lower-level representation a path within the configuration
        class PathRep
        {
        public:
            /// a sequence of element names which make up a path
            typedef std::vector<Name> Components;
            /// a (read-only) iterator to the element names which make up a path
            typedef Components::const_iterator Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef Components::iterator MutatingIterator;
        public:
            /// construct a path consisting of the components of <var>aComponents</var>
            explicit PathRep(Components const& sComponents);

            /// build the path that results from appending <var>aRelativePath</var> to this
            PathRep compose(PathRep const& aRelativePath) const;

            /// construct a path by appending <var>aName</var>
            PathRep child(Name const& aName) const;

            /// build the path that results from dropping the last component off this
            PathRep parent() const;

            /// get the local name (the last component of this path)
            Name getLocalName() const;

            /// get a /-separated string representation of this
            OUString toString() const;
        public:
            /// Get a reference to (or copy of) a collection of all components of this
            Components const& components() const { return m_aComponents; }

            /// check if this is an empty path
            bool isEmpty() const { return m_aComponents.empty(); }

            /// Count the components of this
            Components::size_type countComponents() const { return m_aComponents.size(); }

            /// Insert a component into this path
            void insertComponent(MutatingIterator it, Name aName = Name()) { m_aComponents.insert(it,aName); }
            /// Remove a component from this path
            void removeComponent(MutatingIterator it) { m_aComponents.erase(it); }
            /// Remove all components from this path
            void clearComponents() { m_aComponents.clear(); }

            /// get a STL style iterator to the first component
            Iterator begin() const { return m_aComponents.begin(); }
            /// get a STL style iterator to after the last component
            Iterator end()   const { return m_aComponents.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() { return m_aComponents.begin(); }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate()   { return m_aComponents.end(); }

        // comparison
            // equality (== is primary)
            friend bool operator==(PathRep const& lhs, PathRep const& rhs);
            friend bool operator!=(PathRep const& lhs, PathRep const& rhs)
            { return !(lhs == rhs); }
            // ordering - for maps
            bool before(PathRep const& rhs) const;
            // hashing - for hash maps
            size_t hashCode() const;
        private:
            Components m_aComponents;
        };

    //------------------------------------------------------------------------

        class RelativePath
        {
            PathRep m_aRep;
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// a sequence of element names which make up a path
            typedef PathRep::Components Components;
            /// a (read-only) iterator to the element names which make up a path
            typedef PathRep::Iterator   Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef PathRep::MutatingIterator MutatingIterator;
        public:
        // Construction
            /// construct an empty relative path
            RelativePath();
            /// construct a relative path from <var>aString</var> ignoring any parse errors
            explicit RelativePath(OUString const& aString, NoValidate);
            /// construct a relative path having <var>aName</var> as single component
            explicit RelativePath(Name const& aName);
            /// construct a relative path having <var>aRep</var> as representation
            explicit RelativePath(PathRep const& aRep) : m_aRep(aRep) { init(); }
            /// construct a relative path consisting of the components of <var>aComponents</var>
            explicit RelativePath(Components const& aComponents) : m_aRep(aComponents) { init(); }

            /// build the Path that results from appending <var>aPath</var> to this
            RelativePath compose(RelativePath const& aPath) const;

            /// construct a path to the child of this path named by <var>aName</var>
            RelativePath child(Name const& aName) const;

            /// build the path that results from dropping the last component off this
            RelativePath parent() const;

            /// check if this is an empty path
            bool isEmpty() const { return m_aRep.isEmpty(); }

            /// get the local name (the last component of this path)
            Name getLocalName() const { return m_aRep.getLocalName(); }

            /// get a /-separated string representation of this
            OUString toString() const;
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            Iterator begin() const { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            Iterator end()   const { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate()   { return m_aRep.end_mutate(); }

        // Decomposed access
            /// Get a collection of all components of this
            Components components() const { return m_aRep.components(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            PathRep const& rep() const { return m_aRep; }
        public:
        // comparison operators
            // equality (== is primary)
            friend bool operator==(RelativePath const& lhs, RelativePath const& rhs)
            { return lhs.m_aRep == rhs.m_aRep; }

            // ordering (< is primary)
            friend bool operator< (RelativePath const& lhs, RelativePath const& rhs)
            { return lhs.m_aRep.before(rhs. m_aRep); }

            // hashing
            size_t hashCode() const { return m_aRep.hashCode(); }
        private:
            void init();
        };

    //------------------------------------------------------------------------

        class AbsolutePath
        {
            PathRep m_aRep; enum { REP_OFF = 1 };
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// a sequence of element names which make up a path
            typedef PathRep::Components Components;
            /// a (read-only) iterator to the element names which make up a path
            typedef PathRep::Iterator   Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef PathRep::MutatingIterator MutatingIterator;
        public:
        // Construction
            /// construct a absolute path from <var>aString</var> ignoring any parse errors
            explicit AbsolutePath(OUString const& aString, NoValidate);
            /// construct a absolute path having <var>aRep</var> as representation
            explicit AbsolutePath(PathRep const& aRep) : m_aRep(aRep) { init(); }
            /// construct a absolute path consisting of the components of <var>aComponents</var>
            explicit AbsolutePath(Components const& aComponents) : m_aRep(aComponents) { init(); }

            /// construct an absolute path to the (virtual) hierarchy root
            static  AbsolutePath root();

            /// construct an (otherwise invalid) substitute path for the root of a free-floating node
            static  AbsolutePath detachedRoot();

            /// build the absolute path that results from appending <var>aPath</var> to this
            AbsolutePath compose(RelativePath const& aPath) const;

            /// construct a path to the child of this path named by <var>aName</var>
            AbsolutePath child(Name const& aName) const;

            /// build the path that results from dropping the last component off this
            AbsolutePath parent() const;

            /// check if this is the path to the root node
            bool isRoot() const;

            /// get the local name (the last component of this path)
            Name getLocalName() const { return m_aRep.getLocalName(); }

            Name getModuleName() const { return isRoot() ? Name() : *begin(); }

            /// get a /-separated string representation of this
            OUString toString() const;
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            Iterator begin() const { return m_aRep.begin() + REP_OFF; }
            /// get a STL style iterator to after the last component
            Iterator end()   const { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() { return m_aRep.begin_mutate() + REP_OFF; }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate()   { return m_aRep.end_mutate(); }

        // Decomposed access
            /// Get a collection of all components of this
            Components components() const  { return Components(begin(),end()); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            PathRep const& rep() const { return m_aRep; }
        public:
        // comparison operators
            // equality (== is primary)
            friend bool operator==(AbsolutePath const& lhs, AbsolutePath const& rhs)
            { return lhs.m_aRep == rhs.m_aRep; }

            // ordering (< is primary)
            friend bool operator< (AbsolutePath const& lhs, AbsolutePath const& rhs)
            { return lhs.m_aRep.before(rhs. m_aRep); }

            // hashing
            size_t hashCode() const { return m_aRep.hashCode(); }
        private:
            void init();
        };

        /// distinguishes which kind of path is held in a <type>Path</type> object
        namespace PathType { enum Type { eNAME = 0, eRELATIVE = 1, eABSOLUTE = 2, ePATH = 3 }; }
        //------------------------------------------------------------------------
        class Path
        {
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// distinguishes which kind of path is held in this object
            typedef PathType::Type Type;

            /// a sequence of element names which make up a path
            typedef PathRep::Components Components;
            /// a (read-only) iterator to the element names which make up a path
            typedef PathRep::Iterator   Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef PathRep::MutatingIterator MutatingIterator;
        public:
            /// Parse a /-separated string into a collection of path components (best try, no exceptions)
            static Components parse(OUString const& aString,  Type eType = PathType::ePATH);
        public:
            /// construct a path (of type <var>eType</var>) from <var>aString</var> ignoring any parse errors
            explicit Path(OUString const& aString, NoValidate, Type eType = PathType::ePATH);

            /// construct a path (of type <var>eType</var>) having <var>aRep</var> as representation
            explicit Path(PathRep const& aRep, Type eType = PathType::ePATH);

            /// construct a path (of type <constant>Path::NAME</constant>) containing <var>aName</var>
            explicit Path(Name const& aName);

            /// construct a path (of type <constant>Path::ABSOLUTE</constant>) equal to <var>aPath</var>
            Path(AbsolutePath const& aPath);

            /// construct a path (of type <constant>Path::RELATIVE</constant>) equal to <var>aPath</var>
            Path(RelativePath const& aPath);

            /// build the path (of the same type as this) that results from appending <var>aPath</var> to this
            Path compose(RelativePath const& aPath) const;

            /// construct a path to the child of this path named by <var>aName</var>
            Path child(Name const& aName) const;

            /// build the path that results from dropping the last component off this
            Path parent() const;

            /// tell the type of this (as known)
            Type getType() const { return m_eType; }

            /// get the local name (the last component of this path)
            Name getLocalName() const { return m_aRep.getLocalName(); }

            /// get a /-separated string representation of this
            OUString toString() const;
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            Iterator begin() const { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            Iterator end()   const { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate()   { return m_aRep.end_mutate(); }

        // Decomposed access
            /// Get a collection of all components of this
            Components components() const  { return m_aRep.components(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            PathRep const& rep() const { return m_aRep; }
        public:
        // comparison operators
            // equality (== is primary)
            friend bool operator==(Path const& lhs, Path const& rhs)
            { return lhs.m_aRep == rhs.m_aRep; }
            // hashing
            size_t hashCode() const { return m_aRep.hashCode(); }
        private:
            PathRep m_aRep;
            Type    m_eType;

            void init();
        };



    //------------------------------------------------------------------------
    // Derived comparison operator implementations
    //------------------------------------------------------------------------
        inline bool operator!=(Name const& lhs, Name const& rhs)
        { return !(lhs == rhs); }
        //--------------------------------------------------------------------
        inline bool operator<=(Name const& lhs, Name const& rhs)
        { return !(rhs < lhs); }
        inline bool operator> (Name const& lhs, Name const& rhs)
        { return   rhs < lhs;  }
        inline bool operator>=(Name const& lhs, Name const& rhs)
        { return !(lhs < rhs); }
    //------------------------------------------------------------------------
        inline bool operator!=(AbsolutePath const& lhs, AbsolutePath const& rhs)
        { return !(lhs == rhs); }
        //--------------------------------------------------------------------
        inline bool operator<=(AbsolutePath const& lhs, AbsolutePath const& rhs)
        { return !(rhs < lhs); }
        inline bool operator> (AbsolutePath const& lhs, AbsolutePath const& rhs)
        { return   rhs < lhs;  }
        inline bool operator>=(AbsolutePath const& lhs, AbsolutePath const& rhs)
        { return !(lhs < rhs); }

    //------------------------------------------------------------------------
        inline bool operator!=(RelativePath const& lhs, RelativePath const& rhs)
        { return !(lhs == rhs); }
        //--------------------------------------------------------------------
        inline bool operator<=(RelativePath const& lhs, RelativePath const& rhs)
        { return !(rhs < lhs); }
        inline bool operator> (RelativePath const& lhs, RelativePath const& rhs)
        { return   rhs < lhs;  }
        inline bool operator>=(RelativePath const& lhs, RelativePath const& rhs)
        { return !(lhs < rhs); }
    //------------------------------------------------------------------------
        inline bool operator!=(Path const& lhs, Path const& rhs)
        { return !(lhs == rhs); }
    //------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNAME_HXX_
