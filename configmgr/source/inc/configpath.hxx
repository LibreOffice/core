/*************************************************************************
 *
 *  $RCSfile: configpath.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:56 $
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif  INCLUDED_VECTOR

namespace configmgr
{
    namespace configuration
    {
    //------------------------------------------------------------------------
        using rtl::OUString;
    //------------------------------------------------------------------------
        /// A tag struct for disabling validity checking on arguments
        namespace argument { struct NoValidate {}; }

        class Name;
        class AbsolutePath;
        class RelativePath;
    //------------------------------------------------------------------------
        namespace Path { struct PackageOnly; }

        /// represents a name for a node in the configuration
        class Name
        {
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// construct a Name from a String (internal use only - use creation wrapper functions)
            explicit Name(OUString const& aString, Path::PackageOnly) SAL_THROW(());

        public:
            /// construct an empty Name
            Name() SAL_THROW(()) : m_sRep() {}

            /// check whether this is an empty Name
            bool isEmpty() const SAL_THROW(()) { return m_sRep.getLength() == 0; }

            /// get a string representation of this Name
            const OUString& toString() const SAL_THROW(()) { return m_sRep; }

        public:
        // comparison operators
            // equality (== is primary)
            friend bool operator==(Name const& lhs, Name const& rhs) SAL_THROW(())
            { return !!(lhs.m_sRep == rhs.m_sRep); }

        // comparison operators
            // ordering (< is primary)
            friend bool operator< (Name const& lhs, Name const& rhs) SAL_THROW(())
            { return !!(lhs.m_sRep < rhs.m_sRep); }

        // hashing support
            size_t hashCode() const SAL_THROW(()) { return m_sRep.hashCode(); }
        private:
            OUString m_sRep;
        };
        //--------------------------------------------------------------------

        /** check if this is a well-formed name for a
            config Node (excluding set elements)
        */
        bool isSimpleName(OUString const& sName) SAL_THROW(());

        /** check if this is a well-formed name for a
            config Node (excluding set elements)
        */
        inline
        bool isSimpleName(Name const& sName) SAL_THROW(())
        { return isSimpleName(sName.toString()); }

        /** make a <type>Name</type> out of <var>sName</var>
            without full Validation.
        */
        Name makeName(OUString const& sName, argument::NoValidate) SAL_THROW(());

        /** make a <type>Name</type> out of <var>sName</var>,
            which should be used for a config Node (excluding set elements)
        */
        Name makeNodeName(OUString const& sName, argument::NoValidate) SAL_THROW(());

        /** make a <type>Name</type> out of <var>sName</var>,
            which should be used for a config set elements
        */
        Name makeElementName(OUString const& sName, argument::NoValidate) SAL_THROW(());

        /** make a <type>Name</type> out of <var>sName</var>,
            validating that it can be used for a config Node (excluding set elements)
            or template name.
            @throws InvalidName
                if the name is not valid for that purpose
        */
        Name validateNodeName(OUString const& sName);

        /** make a <type>Name</type> out of <var>sName</var>
            validating that it can be used for a config set element
            @throws InvalidName
                if the name is not valid for that purpose
        */
        Name validateElementName(OUString const& sName);
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
        namespace Path
        {
        //------------------------------------------------------------------------
            /// tag for disabling validity checking on arguments
            using argument::NoValidate;

        //------------------------------------------------------------------------

            class Component
            {
                /// holds the contents of this path component
                Name m_aName;
            public:
                /// construct a path component from a string, without any validation
                Component(OUString const& _sName, PackageOnly) SAL_THROW(());
                /// construct a path component from a Name, without any validation
                Component(Name const& _aName, PackageOnly) SAL_THROW(());

                /// is this component an empty name ?
                bool isEmpty()    const SAL_THROW(()) { return m_aName.isEmpty(); }
                /// is this component a simple name ?
                bool isSimpleName()    const SAL_THROW(());
                /// get the inner name for this component
                Name getName()         const SAL_THROW(());
                /// get the embedded type name for this component (if any)
                Name getTypeName()     const SAL_THROW(());
                /// get inner name and the embedded type name (if any) for this component
                bool splitCompositeName(Name& _rName, Name& _rType) const SAL_THROW(());
                /// get inner name and the embedded type name (if any) for this component as strings
                bool splitCompositeName(OUString& _rName, OUString& _rType) const SAL_THROW(());

                /// get the contents of this as string (unparsed).
                OUString toPathString() const SAL_THROW(()) { return m_aName.toString(); }

                // hashing - for hash maps. compatible to equiv or matches
                size_t hashCode() const SAL_THROW(())
                { return this->getName().hashCode(); }

                /// get the contents of this as a Name (unparsed). Use with care !
                Name const& getInternalName() const SAL_THROW(()) { return m_aName; }

            };

        //-------------------------------------------------------------------------

            /// compare taking type wildcards into account
            bool matches(Component const& lhs,Component const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool before(Component const& lhs,Component const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool equiv(Component const& lhs,Component const& rhs) SAL_THROW(());
        //-------------------------------------------------------------------------

            /// construct a empty path component
            Component makeEmptyComponent() SAL_THROW(());

        //-------------------------------------------------------------------------
            /// construct a path component from a name, validating it as simple name
            Component wrapSimpleName(Name const& _aName);

            /// construct a path component from a type and element name, using a wildcard if no type is available
            Component makeCompositeName(Name const& _aElementName, Name const& _aTypeName);

        //-------------------------------------------------------------------------
            /// construct a path component from a string, validating it as simple name
            Component wrapSimpleName(OUString const& _sName);

            /// construct a path component from a type and element name as strings, using a wildcard if no type is available
            Component makeCompositeName(OUString const& _sElementName, OUString const& _sTypeName);

    //-----------------------------------------------------------------------------
            /// construct a composite path component from a element name or string, using a wildcard type
            template <class NameRep>
            Component wrapElementName(NameRep const& _aElementName) SAL_THROW(())
            {
                return makeCompositeName(_aElementName, NameRep());
            }

        //-------------------------------------------------------------------------
            /// construct a path component from an arbitrary na,e or string
            template <class NameRep>
            Component wrapSafeName(NameRep const& _aName) SAL_THROW(())
            {
                return isSimpleName(_aName) ? wrapSimpleName(_aName) : wrapElementName(_aName);
            }

    //-----------------------------------------------------------------------------
            /** lower-level representation of a path within the configuration
                <p>Keeps the data in a vector of names in reverse order! </P>
            */
            class Rep
            {
            public:
                /// a sequence of element names which make up a path
                typedef std::vector<Component> Components;
                /// a (read-only) iterator to the element names which make up a path
                typedef Components::const_reverse_iterator Iterator;
                /// a (mutating) iterator to the element names which make up a path
                typedef Components::reverse_iterator MutatingIterator;
                /// type used to specify a component count
                typedef Components::size_type size_type;

            public:
                /// construct an empty path
                Rep() SAL_THROW(()) : m_aComponents() {}

                /// construct a path consisting of a single component <var>_aName</var>
                explicit Rep(Component const& _aName) SAL_THROW(()) : m_aComponents(1,_aName) {}

                /// construct a path consisting of a path subrange
                explicit Rep(Iterator const& _first, Iterator const& _last)
                : m_aComponents(_last.base(), _first.base()) {}

                /// swap contents with another instance
                void swap(Rep& _aOther) SAL_THROW(()) { m_aComponents.swap(_aOther.m_aComponents); }

                /// modify a path by prepending <var>aName</var>
                void prepend(Component const& _aName) SAL_THROW(()) { m_aComponents.push_back(_aName); }

                /// modify a path by prepending <var>aName</var>
                void prepend(Rep const& _aOther) SAL_THROW(());

                /// get the local name (the last component of this path)
                Component const& getLocalName() const { check_not_empty(); return m_aComponents.front(); }

                /// get the next name (the first component of this path)
                Component const& getFirstName() const { check_not_empty(); return m_aComponents.back(); }

                /// set this to the remainder after the first name (drop the first component of this path)
                void dropFirstName() { check_not_empty(); m_aComponents.pop_back(); }

                /// get a /-separated string representation of this
                OUString toString(bool _bAbsolute) const SAL_THROW(());

            public:
                /// check if this is an empty path
                bool isEmpty() const SAL_THROW(()) { return m_aComponents.empty(); }

                /// Count the components of this
                size_type countComponents() const SAL_THROW(()) { return m_aComponents.size(); }

                /// Insert a component into this path
                void insertComponent(MutatingIterator _it, Component _aName)
                { m_aComponents.insert(_it.base(),_aName); }

                /// Remove a component from this path
                void removeComponent(MutatingIterator _it) { m_aComponents.erase(_it.base()); }

                /// Remove all components from this path
                void clearComponents() SAL_THROW(()) { m_aComponents.clear(); }

                /// get a STL style iterator to the first component
                Iterator begin() const SAL_THROW(()) { return m_aComponents.rbegin(); }
                /// get a STL style iterator to after the last component
                Iterator end()   const SAL_THROW(()) { return m_aComponents.rend(); }

                /// get a STL style iterator to the first component
                MutatingIterator begin_mutate() SAL_THROW(()) { return m_aComponents.rbegin(); }
                /// get a STL style iterator to after the last component
                MutatingIterator end_mutate() SAL_THROW(())   { return m_aComponents.rend(); }

                // hashing - for hash maps
                size_t hashCode() const SAL_THROW(());

                /// preflight check for operations that require a non-empty path
                void check_not_empty() const;

            private:
                Components m_aComponents;
            };
        //------------------------------------------------------------------------

            /// compare taking type wildcards into account
            bool matches(Rep const& lhs,Rep const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool before(Rep const& lhs,Rep const& rhs) SAL_THROW(());

            /// compare by inner names only
            bool equiv(Rep const& lhs,Rep const& rhs) SAL_THROW(());
        //------------------------------------------------------------------------

            /// check a path for a prefix
            bool hasMatchingPrefix(Rep const& _aPath,Rep const& _aPrefix) SAL_THROW(());

            /// remove a prefix from a path. Throws InvalidName if it isn't a prefix
            Rep stripMatchingPrefix(Rep const& _aPath,Rep const& _aPrefix);
        //------------------------------------------------------------------------

            /// a (read-only) iterator to the element names which make up a path
            typedef Rep::Iterator Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef Rep::MutatingIterator MutatingIterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef Rep::size_type size_type;
        //------------------------------------------------------------------------

            /// distinguishes which kind of path is present in a string
            bool isAbsolutePath(OUString const& _sPath);
        //------------------------------------------------------------------------
        }
    //------------------------------------------------------------------------

        class RelativePath
        {
            Path::Rep m_aRep;
        public:
            /// a (read-only) iterator to the element names which make up a path
            typedef Path::Iterator  Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef Path::MutatingIterator MutatingIterator;
        public:
        // Construction
            /// construct a relative path from <var>aString</var> throwing InvalidName for parse errors
            static  RelativePath parse(OUString const& _aString);

            /// construct an empty relative path
            RelativePath() SAL_THROW(()) : m_aRep() { init(); }

            /// construct a relative path having <var>aRep</var> as representation
            explicit RelativePath(Path::Rep const& _aRep)
            : m_aRep(_aRep) { init(); }

            /// CONVERSION: construct a relative path having <var>aName</var> as single component
            RelativePath(Path::Component const& _aName) SAL_THROW(());

            /// build the Path that results from appending <var>aPath</var> to this
            RelativePath compose(RelativePath const& _aPath) const SAL_THROW(());

            /// check if this is an empty path
            bool isEmpty() const SAL_THROW(()) { return m_aRep.isEmpty(); }

            /// Count the components of this
            Path::size_type getDepth() const SAL_THROW(()) { return m_aRep.countComponents(); }

            /// get the local name (the last component of this path)
            Path::Component const& getLocalName() const { return m_aRep.getLocalName(); }

            /// get the local name (the first component of this path)
            Path::Component const& getFirstName() const { return m_aRep.getFirstName(); }

            /// set this to the remainder of this path after the first name (drop the first component of this path)
            void dropFirstName() { m_aRep.dropFirstName(); }

            /// get a /-separated string representation of this
            OUString toString() const SAL_THROW(());
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            Iterator begin() const SAL_THROW(()) { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            Iterator end()   const SAL_THROW(()) { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() SAL_THROW(()) { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate() SAL_THROW(())   { return m_aRep.end_mutate(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            Path::Rep const& rep() const SAL_THROW(()) { return m_aRep; }

        private:
            void init();
        };

        /// compare taking type wildcards into account
        inline bool matches(RelativePath const& lhs,RelativePath const& rhs) SAL_THROW(())
        { return Path::matches(lhs.rep(),rhs.rep()); }

    //------------------------------------------------------------------------

        class AbsolutePath
        {
            Path::Rep m_aRep;
        public:
            /// A dummy parameter for disabling validity checking on arguments
            typedef argument::NoValidate NoValidate;

            /// a (read-only) iterator to the element names which make up a path
            typedef Path::Iterator  Iterator;
            /// a (mutating) iterator to the element names which make up a path
            typedef Path::MutatingIterator MutatingIterator;
        public:
        // Construction
            /// construct a absolute path from <var>aString</var> throwing InvalidName for parse errors
            static AbsolutePath parse(OUString const& _aString);

            /// construct a absolute path to a whole module (toplevel)
            static AbsolutePath makeModulePath(Name const& _aModuleName)
            { return AbsolutePath( Path::Rep( Path::wrapSimpleName(_aModuleName) ) ); }

            /// construct a absolute path to a whole module (toplevel)
            static AbsolutePath makeModulePath(Name const& _aModuleName, NoValidate) SAL_THROW(());

            /// construct a absolute path to a whole module (toplevel) without error checking
            static AbsolutePath makeModulePath(OUString const& _aString, NoValidate) SAL_THROW(());

            /// construct an absolute path to the (virtual) hierarchy root
            static  AbsolutePath root() SAL_THROW(());

            /// construct an (otherwise invalid) substitute path for the root of a free-floating node
            static  AbsolutePath detachedRoot() SAL_THROW(());

            /// construct a absolute path having <var>aRep</var> as representation
            explicit AbsolutePath(Path::Rep const& _aRep) SAL_THROW(())
            : m_aRep(_aRep) { init(); }

            /// build the absolute path that results from appending <var>aPath</var> to this
            AbsolutePath compose(RelativePath const& _aPath) const SAL_THROW(());

            /// build the absolute path that results from removing the last component of this
            AbsolutePath getParentPath() const;

            /// check if this is the path to the (imaginary) root node
            bool isRoot() const SAL_THROW(()) { return m_aRep.isEmpty(); }

            /// check if this is a path to a detached node
            bool isDetached() const SAL_THROW(());

            /// get the local name (the last component of this path)
            Path::Component const& getLocalName() const { return m_aRep.getLocalName(); }

            Name const & getModuleName() const { return m_aRep.getFirstName().getInternalName(); }

            /// get a /-separated string representation of this
            OUString toString() const SAL_THROW(());

            /// Count the components of this
            Path::size_type getDepth() const SAL_THROW(()) { return m_aRep.countComponents(); }
        public:
        // Iteration support
            /// get a STL style iterator to the first component
            Iterator begin() const SAL_THROW(()) { return m_aRep.begin(); }
            /// get a STL style iterator to after the last component
            Iterator end()   const SAL_THROW(()) { return m_aRep.end(); }

            /// get a STL style iterator to the first component
            MutatingIterator begin_mutate() SAL_THROW(()) { return m_aRep.begin_mutate(); }
            /// get a STL style iterator to after the last component
            MutatingIterator end_mutate() SAL_THROW(())   { return m_aRep.end_mutate(); }

        // Direct access - 'package' visible
            /// Get a reference to (or copy of) the internal PathRep of this
            Path::Rep const& rep() const SAL_THROW(()) { return m_aRep; }
        private:
            void init() SAL_THROW(());
        };

        /// compare taking type wildcards into account
        inline bool matches(AbsolutePath const& lhs,AbsolutePath const& rhs) SAL_THROW(())
        { return Path::matches(lhs.rep(),rhs.rep()); }

    //------------------------------------------------------------------------
    // Derived comparison operator implementations
    //------------------------------------------------------------------------
        inline bool operator!=(Name const& lhs, Name const& rhs) SAL_THROW(())
        { return !(lhs == rhs); }
        //--------------------------------------------------------------------
        inline bool operator<=(Name const& lhs, Name const& rhs) SAL_THROW(())
        { return !(rhs < lhs); }
        inline bool operator> (Name const& lhs, Name const& rhs) SAL_THROW(())
        { return   rhs < lhs;  }
        inline bool operator>=(Name const& lhs, Name const& rhs) SAL_THROW(())
        { return !(lhs < rhs); }
    //------------------------------------------------------------------------

        namespace Path
        {
    //------------------------------------------------------------------------
            template <class PathClass>
            bool hasPrefix(PathClass const& _aPath, PathClass const& _aPrefix) SAL_THROW(())
            {
                return hasMatchingPrefix(_aPath.rep(),_aPrefix.rep() );
            }
    //------------------------------------------------------------------------

            template <class PathClass>
            RelativePath stripPrefix(PathClass const& _aPath, PathClass const& _aPrefix)
            {
                return RelativePath( stripMatchingPrefix(_aPath.rep(),_aPrefix.rep()) );
            }
    //------------------------------------------------------------------------

    // STL Helpers
    //------------------------------------------------------------------------

            /// a weak strict ordering considering only the name part
            struct Before
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return before(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return before(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return before(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return before(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------

            /// an equality relation considering only the name part (compatible to Before)
            struct Equiv
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return equiv(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return equiv(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return equiv(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return equiv(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------

            /// a hash generator (compatible to Equiv and Before)
            struct Hash
            {
                size_t operator()(Component const& _aObject) const SAL_THROW(())
                { return _aObject.hashCode(); }
                size_t operator()(Rep const& _aObject) const SAL_THROW(())
                { return _aObject.hashCode(); }
                size_t operator()(AbsolutePath const& _aObject) const SAL_THROW(())
                { return _aObject.rep().hashCode(); }
                size_t operator()(RelativePath const& _aObject) const SAL_THROW(())
                { return _aObject.rep().hashCode(); }
            };
    //------------------------------------------------------------------------
            /// a binary predicate that is not (!) an equivalence relation

            struct Matches
            {
                bool operator()(Component const& lhs, Component const& rhs) const SAL_THROW(())
                { return matches(lhs,rhs); }
                bool operator()(Rep const& lhs, Rep const& rhs) const SAL_THROW(())
                { return matches(lhs,rhs); }
                bool operator()(AbsolutePath const& lhs, AbsolutePath const& rhs) const SAL_THROW(())
                { return matches(lhs.rep(),rhs.rep()); }
                bool operator()(RelativePath const& lhs, RelativePath const& rhs) const SAL_THROW(())
                { return matches(lhs.rep(),rhs.rep()); }
            };
    //------------------------------------------------------------------------
        }
    //------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNAME_HXX_
