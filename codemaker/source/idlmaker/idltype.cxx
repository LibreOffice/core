/*************************************************************************
 *
 *  $RCSfile: idltype.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mm $ $Date: 2000-09-21 07:43:11 $
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

#include <stdio.h>
#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_STRBUF_HXX_
#include    <rtl/strbuf.hxx>
#endif

#include    "idltype.hxx"
#include    "idloptions.hxx"

using namespace rtl;

//*************************************************************************
// IdlType
//*************************************************************************
IdlType::IdlType(TypeReader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : m_inheritedMemberCount(0)
    , m_indentLength(0)
    , m_typeName(typeName)
    , m_name(typeName.getToken(typeName.getTokenCount('/') - 1, '/'))
    , m_reader(typeReader)
    , m_typeMgr((TypeManager&)typeMgr)
    , m_dependencies(typeDependencies)
{
}

IdlType::~IdlType()
{

}

sal_Bool IdlType::dump(IdlOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".idl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}
sal_Bool IdlType::dumpDependedTypes(IdlOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_True;

    TypeUsingSet usingSet(m_dependencies.getDependencies(m_typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();
    OString typeName;
    sal_uInt32 index = 0;
    while (iter != usingSet.end())
    {
        typeName = (*iter).m_type;
        if ((index = typeName.lastIndexOf(']')) > 0)
            typeName = typeName.copy(index + 1);

        if (getBaseType(typeName).getLength() == 0)
        {
            if (!produceType(typeName,
                                m_typeMgr,
                             m_dependencies,
                             pOptions))
            {
                fprintf(stderr, "%s ERROR: %s\n",
                        pOptions->getProgramName().getStr(),
                        OString("cannot dump Type '" + typeName + "'").getStr());
                exit(99);
            }
        }
        iter++;
    }

    return ret;
}

OString IdlType::dumpHeaderDefine(FileStream& o, sal_Char* prefix )
{
    if (m_typeName.equals("/"))
    {
        m_typeName = "global";
    }

    sal_uInt32 length = 3 + m_typeName.getLength() + strlen(prefix);

    OStringBuffer tmpBuf(length);

    tmpBuf.append('_');
    tmpBuf.append(m_typeName);
    tmpBuf.append('_');
    tmpBuf.append(prefix);
    tmpBuf.append('_');

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toUpperCase());

    o << "#ifndef " << tmp << "\n#define " << tmp << endl;

    return tmp;
}

void IdlType::dumpDefaultHIncludes(FileStream& o)
{
}

void IdlType::dumpInclude(FileStream& o, const OString& genTypeName, const OString& typeName, sal_Char* prefix )
{
    sal_uInt32 length = 3+ m_typeName.getLength() + strlen(prefix);

    OStringBuffer tmpBuf(length);

    tmpBuf.append('_');
    tmpBuf.append(typeName);
    tmpBuf.append('_');
    tmpBuf.append(prefix);
    tmpBuf.append('_');

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toUpperCase());

    length = 1 + typeName.getLength() + strlen(prefix);

    tmpBuf.ensureCapacity(length);
    tmpBuf.append(typeName);
    tmpBuf.append('.');
    tmpBuf.append(prefix);

    o << "#ifndef " << tmp << "\n#include <";
    tmp = tmpBuf.makeStringAndClear();

    sal_Int32 nSlashes = genTypeName.getTokenCount( '/');
    for( sal_Int32 i = 1; i < nSlashes; i++ )
        o << "../";
    //o << "c:/temp/";
    o << tmp;
    o << ">\n#endif\n";
}

void IdlType::dumpDepIncludes(FileStream& o, const OString& typeName, sal_Char* prefix)
{
    TypeUsingSet usingSet(m_dependencies.getDependencies(typeName));

    TypeUsingSet::const_iterator iter = usingSet.begin();

    OString     sPrefix(OString(prefix).toUpperCase());
    sal_uInt32  index = 0;
    sal_uInt32  seqNum = 0;
    OString     relType;
    while (iter != usingSet.end())
    {
        index = (*iter).m_type.lastIndexOf(']');
        seqNum = (index > 0 ? ((index+1) / 2) : 0);

        relType = (*iter).m_type;
        if (index > 0)
            relType = relType.copy(index+1);


        OString defPrefix("IDL");

        if (getBaseType(relType).getLength() == 0 &&
            m_typeName != relType)
        {
            if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
            {
                if (!((*iter).m_use & TYPEUSE_SUPER))
                {
                    o << endl;
                    dumpNameSpace(o, sal_True, sal_False, relType);
                    o << "\ninterface " << scopedName(m_typeName, relType, sal_True) << ";\n";
                    dumpNameSpace(o, sal_False, sal_False, relType);
                    o << "\n\n";
                }
            }
            dumpInclude(o, typeName, relType, prefix);
        }
        else if (relType == "type")
        {
            o << "module CORBA {\n"
              << "\tinterface TypeCode;\n"
              << "};\n\n";
        }

        if( seqNum != 0 )
        {
            // write typedef for sequences to support Rational Rose 2000 import
            OString aST = relType;
            OString aScope;
            dumpNameSpace( o, sal_True, sal_False, relType );
            for( sal_Int32 i = 0; i < seqNum; i++ )
            {
                o << "typedef sequence< " << scopedName("", aST) << " > ";

                if( i == 0 )
                {
                    aST = aST.replace( '/', '_' );
                    aST = aST.replace( ' ', '_' );
                }
                aST = aST + "_Sequence" ;
                o << aST << ";\n";
            }
            dumpNameSpace( o, sal_False, sal_False, relType );
        }

        iter++;
    }
}

void IdlType::dumpNameSpace(FileStream& o, sal_Bool bOpen, sal_Bool bFull, const OString& type)
{
    OString typeName(type);
    sal_Bool bOneLine = sal_True;
    if (typeName.getLength() == 0)
    {
        typeName = m_typeName;
        bOneLine = sal_False;
    }

    if (typeName == "/")
        return;

    sal_uInt32 count = typeName.getTokenCount('/');

    if (count == 1 && !bFull)
        return;

    if (!bFull) count--;

    if (bOpen)
    {
        for (int i=0; i < count; i++)
        {
            o << "module " << typeName.getToken(i, '/');
            if (bOneLine)
                o << " { ";
            else
                 o << "\n{\n";
        }
    } else
    {
        for (int i=count-1; i >= 0; i--)
        {
            o << "};";
            if (bOneLine)
                o << " ";
            else
                 o << " /* " << typeName.getToken(i, '/') << "*/\n";
        }
    }
}


sal_uInt32 IdlType::getMemberCount()
{
    sal_uInt32 count = m_reader.getMethodCount();

    sal_uInt32 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            count++;
    }
    return count;
}

sal_uInt32 IdlType::checkInheritedMemberCount(const TypeReader* pReader)
{
    sal_Bool bSelfCheck = sal_True;
    if (!pReader)
    {
        bSelfCheck = sal_False;
        pReader = &m_reader;
    }

    sal_uInt32 count = 0;
    OString superType(pReader->getSuperTypeName());
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));
        if ( aSuperReader.isValid() )
        {
            count = checkInheritedMemberCount(&aSuperReader);
        }
    }

    if (bSelfCheck)
    {
        count += pReader->getMethodCount();
        sal_uInt32 fieldCount = pReader->getFieldCount();
        RTFieldAccess access = RT_ACCESS_INVALID;
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = pReader->getFieldAccess(i);

            if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            {
                count++;
            }
        }
    }

    return count;
}

sal_uInt32 IdlType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}


void IdlType::dumpType(FileStream& o, const OString& type )
    throw( CannotDumpException )
{
    OString sType(checkRealBaseType(type, sal_True));
    sal_uInt32 index = sType.lastIndexOf(']');
    sal_uInt32 seqNum = (index > 0 ? ((index+1) / 2) : 0);

    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    int i;
/*
    for (i=0; i < seqNum; i++)
    {
        //o << "sequence< ";
    }
*/
    switch (typeClass)
    {
        case RT_TYPE_INVALID:
            {
                OString tmp(getBaseType(relType));
                if (tmp.getLength() > 0)
                {
                    tmp = tmp.replace( ' ', '_' );
                    o << tmp;
                } else
                    throw CannotDumpException("Unknown type '" + relType + "', incomplete type library.");
            }
            break;
        case RT_TYPE_INTERFACE:
        case RT_TYPE_STRUCT:
        case RT_TYPE_ENUM:
        case RT_TYPE_TYPEDEF:
        case RT_TYPE_EXCEPTION:
                if( seqNum )
                {
                    OString aST = relType.replace( '/', '_' );
                    aST = aST.replace( ' ', '_' );
                    o << aST;
                }
                else
                    o << scopedName(m_typeName, relType);
            break;
    }

    for (i=0; i < seqNum; i++)
    {
        //o << " >";
        // use typedef for sequences to support Rational Rose 2000 import
        o << "_Sequence";
    }
}

OString IdlType::getBaseType(const OString& type)
{
    if (type.equals("long"))
        return type;
    if (type.equals("short"))
        return type;
    if (type.equals("hyper"))
        return "long long";
    if (type.equals("string"))
        return "string";
    if (type.equals("boolean"))
        return type;
    if (type.equals("char"))
        return "char";
    if (type.equals("byte"))
        return "byte";
    if (type.equals("any"))
        return type;
    if (type.equals("type"))
        return "CORBA::TypeCode";
    if (type.equals("float"))
        return type;
    if (type.equals("double"))
        return type;
    if (type.equals("octet"))
        return type;
    if (type.equals("void"))
        return type;
    if (type.equals("unsigned long"))
        return type;
    if (type.equals("unsigned short"))
        return type;
    if (type.equals("unsigned hyper"))
        return "unsigned long long";

    return OString();
}

void IdlType::dumpIdlGetType(FileStream& o, const OString& type, sal_Bool bDecl, IdlTypeDecl eDeclFlag)
{
    OString sType( checkRealBaseType(type, sal_True) );
    sal_uInt32 index = sType.lastIndexOf(']');
    OString relType = (index > 0 ? (sType).copy(index+1) : type);

    if (eDeclFlag == CPPUTYPEDECL_ONLYINTERFACES)
    {
         if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
        {
            o << indent() << "getIdlType( (";
            dumpType(o, type);
            o << "*)0 )";

            if (bDecl)
                o << ";\n";
        }
    } else
    {
        if (isBaseType(type))
        {
            return;
        } else
        {
            if (eDeclFlag == CPPUTYPEDECL_NOINTERFACES &&
                m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
                return;

//          if (m_typeMgr.getTypeClass(type) == RT_TYPE_TYPEDEF)
//          {
//              o << indent() << "get_" << type.replace('/', '_') << "_Type()";
//          } else
//          {
                o << indent() << "getIdlType( (";
                dumpType(o, type);
                o << "*)0 )";
//          }
        }
        if (bDecl)
            o << ";\n";
    }
}

BASETYPE IdlType::isBaseType(const OString& type)
{
    if (type.equals("long"))
        return BT_LONG;
    if (type.equals("short"))
        return BT_SHORT;
    if (type.equals("hyper"))
        return BT_HYPER;
    if (type.equals("string"))
        return BT_STRING;
    if (type.equals("boolean"))
        return BT_BOOLEAN;
    if (type.equals("char"))
        return BT_CHAR;
    if (type.equals("byte"))
        return BT_BYTE;
    if (type.equals("any"))
        return BT_ANY;
    if (type.equals("float"))
        return BT_FLOAT;
    if (type.equals("double"))
        return BT_DOUBLE;
    if (type.equals("void"))
        return BT_VOID;
    if (type.equals("unsigned long"))
        return BT_UNSIGNED_LONG;
    if (type.equals("unsigned short"))
        return BT_UNSIGNED_SHORT;
    if (type.equals("unsigned hyper"))
        return BT_UNSIGNED_HYPER;

    return BT_INVALID;
}

OString IdlType::checkSpecialIdlType(const OString& type)
{
    OString baseType(type);

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

    RegistryKey     key;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        isTypeDef = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    TypeReader      reader;

    while (isTypeDef)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
                baseType = reader.getSuperTypeName();
            else
                isTypeDef = sal_False;
        } else
        {
            break;
        }
    }

    return baseType;
}

OString IdlType::checkRealBaseType(const OString& type, sal_Bool bResolveTypeOnly)
{
    sal_uInt32 index = type.lastIndexOf(']');
    OString baseType = (index > 0 ? ((OString)type).copy(index+1) : type);
    OString seqPrefix = (index > 0 ? ((OString)type).copy(0, index+1) : OString());

    RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

    RegistryKey     key;
    sal_uInt8*      pBuffer=NULL;
    RTTypeClass     typeClass;
    sal_Bool        mustBeChecked = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    TypeReader      reader;

    while (mustBeChecked)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
            {
                baseType = reader.getSuperTypeName();
                index = baseType.lastIndexOf(']');
                  if (index > 0)
                {
                    seqPrefix += baseType.copy(0, index+1);
                    baseType = baseType.copy(index+1);
                }
            } else
                mustBeChecked = sal_False;
        } else
        {
            break;
        }
    }

    if ( bResolveTypeOnly )
        baseType = seqPrefix + baseType;

    return baseType;
}

void IdlType::dumpConstantValue(FileStream& o, sal_uInt16 index)
{
    RTConstValue constValue = m_reader.getFieldConstValue(index);

    switch (constValue.m_type)
    {
        case RT_TYPE_BOOL:
            if (constValue.m_value.aBool)
                o << "true";
            else
                o << "false";
            break;
        case RT_TYPE_BYTE:
            {
                char tmp[16];
                sprintf(tmp, "0x%x", (sal_Int8)constValue.m_value.aByte);
                o << tmp;
            }
            break;
        case RT_TYPE_INT16:
            o << constValue.m_value.aShort;
            break;
        case RT_TYPE_UINT16:
            o << constValue.m_value.aUShort;
            break;
        case RT_TYPE_INT32:
            o << constValue.m_value.aLong;
            break;
        case RT_TYPE_UINT32:
            o << constValue.m_value.aULong;
            break;
        case RT_TYPE_FLOAT:
            o << constValue.m_value.aFloat;
            break;
        case RT_TYPE_DOUBLE:
            o << constValue.m_value.aDouble;
            break;
        case RT_TYPE_STRING:
            {
                ::rtl::OUString aUStr(constValue.m_value.aString);
                ::rtl::OString aStr = ::rtl::OUStringToOString(aUStr, RTL_TEXTENCODING_ASCII_US);
                o << "\"" << aStr.getStr() << "\")";
            }
            break;
    }
}

void IdlType::inc(sal_uInt32 num)
{
    m_indentLength += num;
}

void IdlType::dec(sal_uInt32 num)
{
    if (m_indentLength - num < 0)
        m_indentLength = 0;
    else
        m_indentLength -= num;
}

OString IdlType::indent()
{
    OStringBuffer tmp(m_indentLength);

    for (int i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

OString IdlType::indent(sal_uInt32 num)
{
    OStringBuffer tmp(m_indentLength + num);

    for (int i=0; i < m_indentLength + num; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

//*************************************************************************
// InterfaceType
//*************************************************************************
InterfaceType::InterfaceType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
    m_inheritedMemberCount = 0;
    m_hasAttributes = sal_False;
    m_hasMethods = sal_False;
}

InterfaceType::~InterfaceType()
{

}

sal_Bool InterfaceType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "idl");
    o << endl;
    dumpNameSpace(o);

    // write documentation
    OString aDoc = m_reader.getDoku();
    if( aDoc.getLength() )
        o << "/**\n" << aDoc << "\n*/";
    o << "\ninterface " << m_name;

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        o << " : " << scopedName(m_typeName, superType);

    o << "\n{\n";
    inc();

    dumpAttributes(o);
    dumpMethods(o);

    dec();
    o << "};\n\n";

    dumpNameSpace(o, sal_False);

//  o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
//    << "class Type;\n} } } }\n\n";

    o << "#endif /* "<< headerDefine << "*/" << endl;
    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o)
{
    sal_uInt32 fieldCount = m_reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        if (first)
        {
            first = sal_False;
            o << "\n";
        }

        // write documentation
        OString aDoc = m_reader.getFieldDoku(i);
        if( aDoc.getLength() )
            o << "/**\n" << aDoc << "\n*/\n";

        if (access == RT_ACCESS_READONLY)
            o << indent() << "readonly attribute ";
        else
            o << indent() << "attribute ";
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }
}

void InterfaceType::dumpMethods(FileStream& o)
{
    sal_uInt32 methodCount = m_reader.getMethodCount();

    OString methodName, returnType, paramType, paramName;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;

    sal_Bool bRef = sal_False;
    sal_Bool bConst = sal_False;
    sal_Bool bWithRunTimeExcp = sal_True;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodName = m_reader.getMethodName(i);
        returnType = m_reader.getMethodReturnType(i);
        paramCount = m_reader.getMethodParamCount(i);
        excCount = m_reader.getMethodExcCount(i);
        methodMode = m_reader.getMethodMode(i);

        if ( methodName.equals("acquire") || methodName.equals("release") )
        {
            bWithRunTimeExcp = sal_False;
        }

        // write documentation
        OString aDoc = m_reader.getMethodDoku(i);
        if( aDoc.getLength() )
            o << "/**\n" << aDoc << "\n*/\n";

        o << indent();
        dumpType(o, returnType);
        o << " " << methodName << "( ";
        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramName = m_reader.getMethodParamName(i, j);
            paramType = m_reader.getMethodParamType(i, j);
            paramMode = m_reader.getMethodParamMode(i, j);

            switch (paramMode)
            {
                case RT_PARAM_IN:
                    o << "in ";
                    break;
                case RT_PARAM_OUT:
                    o << "out ";
                    break;
                case RT_PARAM_INOUT:
                    o << "inout ";
                    break;
                    break;
            }

            dumpType(o, paramType);
            if( paramName == "Object" )
                o << " _Object";
            else
                o << " " << paramName;

            if (j+1 < paramCount) o << ", ";
        }
        o << " )";

        if( excCount )
        {
            o << " raises(";
            OString excpName;
            sal_Bool bWriteComma = sal_False;
            sal_Bool bRTExceptionWritten = sal_False;
            for (j=0; j < excCount; j++)
            {
                excpName = m_reader.getMethodExcType(i, j);
                if( bWriteComma )
                    o << ", ";
                o << scopedName(m_typeName, excpName);
                bWriteComma = sal_True;

                if(excpName == "com/sun/star/uno/RuntimeException")
                    bRTExceptionWritten = sal_True;
            }

            if ( bWithRunTimeExcp && !bRTExceptionWritten )
            {
                if( bWriteComma )
                    o << ", ";
                o << "::com::sun::star::uno::RuntimeException";
            }

            o << ");\n";
        }
        else if ( bWithRunTimeExcp )
        {
            o << "raises( ::com::sun::star::uno::RuntimeException );\n";
        }
        else
        {
            o << ";\n";
        }
    }
}


sal_uInt32 InterfaceType::getMemberCount()
{
    sal_uInt32 count = m_reader.getMethodCount();

    if (count)
        m_hasMethods = sal_True;

    sal_uInt32 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
        {
            m_hasAttributes = sal_True;
            count++;
        }
    }
    return count;
}

sal_uInt32 InterfaceType::checkInheritedMemberCount(const TypeReader* pReader)
{
    sal_uInt32 cout = 0;
    sal_Bool bSelfCheck = sal_True;
    if (!pReader)
    {
        bSelfCheck = sal_False;
        pReader = &m_reader;
    }

    sal_uInt32 count = 0;
    OString superType(pReader->getSuperTypeName());
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));
        if (aSuperReader.isValid())
        {
            count = checkInheritedMemberCount(&aSuperReader);
        }
    }

    if (bSelfCheck)
    {
        count += pReader->getMethodCount();
        sal_uInt32 fieldCount = pReader->getFieldCount();
        RTFieldAccess access = RT_ACCESS_INVALID;
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = pReader->getFieldAccess(i);

            if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            {
                count++;
            }
        }
    }

    return count;
}

sal_uInt32 InterfaceType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}



//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(TypeReader& typeReader,
                        const OString& typeName,
                       const TypeManager& typeMgr,
                       const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ModuleType::~ModuleType()
{

}

sal_Bool ModuleType::dump(IdlOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpName(m_typeName);

    if (tmpName.equals("/"))
        tmpName = "global";
    else
        tmpName += "/" + m_typeName.getToken(m_typeName.getTokenCount('/') - 1, '/');

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, tmpName, ".idl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}

sal_Bool ModuleType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "idl");
    o << endl;

    dumpNameSpace(o, sal_True, sal_True);
    o << endl;

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
        {
            fieldName = m_reader.getFieldName(i);
            fieldType = m_reader.getFieldType(i);

            o << "const ";
            dumpType(o, fieldType);
            o << " " << fieldName << " = ";
            dumpConstantValue(o, i);
            o << ";\n";
        }
    }

    o << endl;
    dumpNameSpace(o, sal_False, sal_True);
    o << "\n#endif /* "<< headerDefine << "*/" << endl;

    return sal_True;
}

sal_Bool ModuleType::hasConstants()
{
    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST)
            return sal_True;
    }

    return sal_False;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : ModuleType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ConstantsType::~ConstantsType()
{

}

sal_Bool ConstantsType::dump(IdlOptions* pOptions)
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    OString tmpFileName;
    OString hFileName = createFileNameFromType(outPath, m_typeName, ".idl");

    sal_Bool bFileExists = sal_False;
    sal_Bool bFileCheck = sal_False;

    if ( pOptions->isValid("-G") || pOptions->isValid("-Gc") )
    {
        bFileExists = fileExists( hFileName );
        ret = sal_True;
    }

    if ( bFileExists && pOptions->isValid("-Gc") )
    {
        tmpFileName  = createFileNameFromType(outPath, m_typeName, ".tml");
        bFileCheck = sal_True;
    }

    if ( !bFileExists || bFileCheck )
    {
        FileStream hFile;

        if ( bFileCheck )
            hFile.openFile(tmpFileName);
        else
            hFile.openFile(hFileName);

        if(!hFile.isValid())
        {
            OString message("cannot open ");
            message += hFileName + " for writing";
            throw CannotDumpException(message);
        }

        ret = dumpHFile(hFile);

        hFile.closeFile();
        if (ret && bFileCheck)
        {
            ret = checkFileContent(hFileName, tmpFileName);
        }
    }

    return ret;
}

//*************************************************************************
// StructureType
//*************************************************************************
StructureType::StructureType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
}

StructureType::~StructureType()
{

}

sal_Bool StructureType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "idl");
    o << endl;

    dumpNameSpace(o);

    // write documentation
    OString aDoc = m_reader.getDoku();
    if( aDoc.getLength() )
        o << "/**\n" << aDoc << "\n*/";

    o << "\nstruct " << m_name;
    o << "\n{\n";
    inc();

    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        dumpSuperMember(o, superType);

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i=0;

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        // write documentation
        OString aDoc = m_reader.getFieldDoku(i);
        if( aDoc.getLength() )
            o << "/**\n" << aDoc << "\n*/";

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }

    dec();
    o << "};\n\n";

    dumpNameSpace(o, sal_False);

    o << "#endif /* "<< headerDefine << "*/" << endl;

    return sal_True;
}

void StructureType::dumpSuperMember(FileStream& o, const OString& superType)
{
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            dumpSuperMember(o, aSuperReader.getSuperTypeName());

            sal_uInt32      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_Int16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = aSuperReader.getFieldName(i);
                fieldType = aSuperReader.getFieldType(i);

                // write documentation
                OString aDoc = aSuperReader.getFieldDoku(i);
                if( aDoc.getLength() )
                    o << "/**\n" << aDoc << "\n*/";

                o << indent();
                dumpType(o, fieldType);
                o << " ";
                o << fieldName << ";\n";
            }
        }
    }
}

//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(TypeReader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr,
                             const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
}

ExceptionType::~ExceptionType()
{

}

sal_Bool ExceptionType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "idl");
    o << endl;

    dumpNameSpace(o);

    // write documentation
    OString aDoc = m_reader.getDoku();
    if( aDoc.getLength() )
        o << "/**\n" << aDoc << "\n*/";

    o << "\nexception " << m_name;
    o << "\n{\n";
    inc();

    // Write extra member for derived exceptions
    o << indent() << "/*extra member to hold a derived exception */\n";
    o << indent() << "any _derivedException;\n";
    OString superType(m_reader.getSuperTypeName());
    if (superType.getLength() > 0)
        dumpSuperMember(o, superType);

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i = 0;

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = m_reader.getFieldName(i);
        fieldType = m_reader.getFieldType(i);

        // write documentation
        OString aDoc = m_reader.getFieldDoku(i);
        if( aDoc.getLength() )
            o << "/**\n" << aDoc << "\n*/";

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName << ";\n";
    }


    dec();
    o << "};\n\n";

    dumpNameSpace(o, sal_False);

    o << "#endif /* "<< headerDefine << "*/" << endl;

    return sal_True;
}

void ExceptionType::dumpSuperMember(FileStream& o, const OString& superType)
{
    if (superType.getLength() > 0)
    {
        TypeReader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            dumpSuperMember(o, aSuperReader.getSuperTypeName());

            sal_uInt32      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldAccess(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = aSuperReader.getFieldName(i);
                fieldType = aSuperReader.getFieldType(i);

                // write documentation
                OString aDoc = aSuperReader.getFieldDoku(i);
                if( aDoc.getLength() )
                    o << "/**\n" << aDoc << "\n*/";

                o << indent();
                dumpType(o, fieldType);
                o << " ";
                o << fieldName << ";\n";
            }
        }
    }
}

//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(TypeReader& typeReader,
                    const OString& typeName,
                   const TypeManager& typeMgr,
                   const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
}

EnumType::~EnumType()
{

}

sal_Bool EnumType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;

    dumpNameSpace(o);

    // write documentation
    OString aDoc = m_reader.getDoku();
    if( aDoc.getLength() )
        o << "/**\n" << aDoc << "\n*/";

    o << "\nenum " << m_name << "\n{\n";
    inc();

    sal_uInt32      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_uInt32      value=0;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldAccess(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = m_reader.getFieldName(i);
        constValue = m_reader.getFieldConstValue(i);

        if (constValue.m_type == RT_TYPE_INT32)
            value = constValue.m_value.aLong;
        else
            value++;

        /* doesn't work with rational rose 2000
        // write documentation
        OString aDoc = m_reader.getFieldDoku(i);
        if( aDoc.getLength() )
        */
        //  o << "/**\n" << aDoc << "\n*/\n";
        o << indent() << fieldName;
        if( i +1 < fieldCount )
            o << ",\n";
    }

    dec();
    o << "\n};\n\n";

    dumpNameSpace(o, sal_False);

    o << "#endif /* "<< headerDefine << "*/" << endl;

    return sal_True;
}


//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(TypeReader& typeReader,
                             const OString& typeName,
                            const TypeManager& typeMgr,
                            const TypeDependency& typeDependencies)
    : IdlType(typeReader, typeName, typeMgr, typeDependencies)
{
}

TypeDefType::~TypeDefType()
{

}

sal_Bool TypeDefType::dumpHFile(FileStream& o)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "IDL"));
    o << endl;

    dumpDefaultHIncludes(o);
    o << endl;
    dumpDepIncludes(o, m_typeName, "idl");
    o << endl;

    dumpNameSpace(o);

    o << "\ntypedef ";
    dumpType(o, m_reader.getSuperTypeName());
    o << " " << m_name << ";\n\n";

    dumpNameSpace(o, sal_False);

    o << "#endif /* "<< headerDefine << "*/" << endl;

    return sal_True;
}


//*************************************************************************
// produceType
//*************************************************************************
sal_Bool produceType(const OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     IdlOptions* pOptions)
    throw( CannotDumpException )
{
    if (typeDependencies.isGenerated(typeName))
        return sal_True;

    TypeReader reader(typeMgr.getTypeReader(typeName));

    if (!reader.isValid())
    {
        if (typeName.equals("/"))
            return sal_True;
        else
            return sal_False;
    }

    if( !checkTypeDependencies(typeMgr, typeDependencies, typeName))
        return sal_False;

    RTTypeClass typeClass = reader.getTypeClass();
    sal_Bool    ret = sal_False;
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            {
                InterfaceType iType(reader, typeName, typeMgr, typeDependencies);
                ret = iType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = iType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_MODULE:
            {
                ModuleType mType(reader, typeName, typeMgr, typeDependencies);
                if (mType.hasConstants())
                {
                    ret = mType.dump(pOptions);
                    if (ret) typeDependencies.setGenerated(typeName);
//                  ret = mType.dumpDependedTypes(pOptions);
                } else
                {
                    typeDependencies.setGenerated(typeName);
                    ret = sal_True;
                }
            }
            break;
        case RT_TYPE_STRUCT:
            {
                StructureType sType(reader, typeName, typeMgr, typeDependencies);
                ret = sType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = sType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_ENUM:
            {
                EnumType enType(reader, typeName, typeMgr, typeDependencies);
                ret = enType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = enType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_EXCEPTION:
            {
                ExceptionType eType(reader, typeName, typeMgr, typeDependencies);
                ret = eType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = eType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_TYPEDEF:
            {
                TypeDefType tdType(reader, typeName, typeMgr, typeDependencies);
                ret = tdType.dump(pOptions);
                if (ret) typeDependencies.setGenerated(typeName);
                ret = tdType.dumpDependedTypes(pOptions);
            }
            break;
        case RT_TYPE_CONSTANTS:
            {
                ConstantsType cType(reader, typeName, typeMgr, typeDependencies);
                if (cType.hasConstants())
                {
                    ret = cType.dump(pOptions);
                    if (ret) typeDependencies.setGenerated(typeName);
//                  ret = cType.dumpDependedTypes(pOptions);
                } else
                {
                    typeDependencies.setGenerated(typeName);
                    ret = sal_True;
                }
            }
            break;
        case RT_TYPE_SERVICE:
        case RT_TYPE_OBJECT:
            ret = sal_True;
            break;
    }

    return ret;
}

//*************************************************************************
// scopedName
//*************************************************************************
OString scopedName(const OString& scope, const OString& type,
                   sal_Bool bNoNameSpace)
{
    sal_uInt32 count = type.getTokenCount('/');
    sal_uInt32 offset = 0;

    if (count == 1)
        return type;

    if (bNoNameSpace)
        return type.getToken(count - 1, '/');

    OStringBuffer tmpBuf(type.getLength() + count);
    for (int i=0; i < count; i++)
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(i, '/'));
    }

    return tmpBuf.makeStringAndClear();
}

//*************************************************************************
// shortScopedName
//*************************************************************************
OString scope(const OString& scope, const OString& type )
{
    sal_uInt32 count = type.getTokenCount('/');
    sal_uInt32 offset = 0;

    OStringBuffer tmpBuf(type.getLength() + count);
    for (int i=0; i < count -1; i++)
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(i, '/'));
    }

    return tmpBuf.makeStringAndClear();
}


