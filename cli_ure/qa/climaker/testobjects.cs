/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



using System;
using System.Reflection;
using System.Diagnostics;
using uno;

using unoidl.test.cliure.climaker;
//using unoidl.com.sun.star.uno;
using ucss=unoidl.com.sun.star;


/* To create this component, the class context (in this assembly) can be used. When
   createInstanceWithArgumentsAndContext is called on the service manager
   then the arguments are passed into the ctor.
*/
class Component:uno.util.WeakComponentBase, XTest
{
    public Component(ucss.uno.XComponentContext ctx) {
        m_args = new Any[] {new Any(typeof(ucss.uno.XComponentContext), ctx)};
        m_A2 = 0;
        m_A4 = 0;
    }

    public Component(ucss.uno.XComponentContext ctx, uno.Any[] args) {
        m_args = new Any[args.Length + 1];
        m_args[0] = new Any(typeof(ucss.uno.XComponentContext), ctx);
        for (int i = 0; i < args.Length; i ++) {
            m_args[i+1] = args[i];
        }
    }
    
	public Any[] Args {
		get	
		{
            return m_args;
		}
	}

    // XTest
    public int A1 {
        get {
            return m_A1;
        }
        set {
            m_A1 = value;
        }
    }

    public int A2 {
        get {
            return m_A2;
        }
    }

    public int A3 {
        get {
            return m_A3;
        }
        set {
            m_A3 = value;
        }
    }

    public int A4 {
        get {
            return m_A4;
        }
    }

    public bool test() {
        return true;
    }

    public void testOneway()
    {
    }

    public void testExceptions()
    {
    }

    public PolyStruct testPolyStruct(PolyStruct val)
    {
        return val;
    }

    public void inParameters(bool aBool, byte aByte,
                      short aShort, ushort aUShort,
                      int aInt, uint aUInt,
                      long aLong, ulong aULong,
                      float aFloat, double aDouble,
                      char aChar, string aString,
                      Type aType, uno.Any aAny,
                      Enum2 aEnum, Struct1 aStruct,
                      object aXInterface,
                      unoidl.com.sun.star.lang.XComponent aXComponent,
                      bool[] seqBool)
    {
        m_Bool = aBool;
        m_Byte = aByte;
        m_Short = aShort;
        m_UShort = aUShort;
        m_Int = aInt;
        m_UInt = aUInt;
        m_Long = aLong;
        m_ULong = aULong;
        m_Float = aFloat;
        m_Double = aDouble;
        m_Char = aChar;
        m_String = aString;
        m_Type = aType;
        m_Any = aAny;
        m_Enum2 = aEnum;
        m_Struct1 = aStruct;
        m_XInterface = aXInterface;
        m_XComponent = aXComponent;
        m_seqBool = seqBool;

    }

    public void outParameters(out bool aBool, out byte aByte,
                      out short aShort, out ushort aUShort,
                      out int aInt, out uint aUInt,
                      out long aLong, out ulong aULong,
                      out float aFloat, out double aDouble,
                      out char aChar, out string aString,
                      out Type aType, out uno.Any aAny,
                      out Enum2 aEnum, out Struct1 aStruct,
                      out object aXInterface,
                      out unoidl.com.sun.star.lang.XComponent aXComponent,
                      out bool[] seqBool)
    {
        aBool = m_Bool;
        aByte = m_Byte;
        aShort = m_Short;
        aUShort = m_UShort;
        aInt = m_Int;
        aUInt = m_UInt;
        aLong = m_Long;
        aULong = m_ULong;
        aFloat = m_Float;
        aDouble = m_Double;
        aChar = m_Char;
        aString = m_String;
        aType = m_Type;
        aAny = m_Any;
        aEnum = m_Enum2;
        aStruct = m_Struct1;
        aXInterface = m_XInterface;
        aXComponent = m_XComponent;
        seqBool = m_seqBool;
            
    }

    //returns the values which have been set in a previous call
    //to this function or inParameters.
    public void inoutParameters(ref bool aBool, ref byte aByte,
                      ref short aShort, ref ushort aUShort,
                      ref int aInt, ref uint aUInt,
                      ref long aLong, ref ulong aULong,
                      ref float aFloat, ref double aDouble,
                      ref char aChar,  ref string aString,
                      ref Type aType, ref uno.Any aAny,
                      ref Enum2 aEnum, ref Struct1 aStruct,
                      ref object aXInterface,
                      ref unoidl.com.sun.star.lang.XComponent aXComponent,
                      ref bool[] seqBool)
    {
        bool _bool = aBool;
        aBool = m_Bool;
        m_Bool = _bool;

        byte _byte = aByte;
        aByte = m_Byte;
        m_Byte = _byte;
        
        short _short = aShort;
        aShort = m_Short;
        m_Short = _short;

        ushort _ushort = aUShort;
        aUShort = m_UShort;
        m_UShort = _ushort;

        int _int = aInt;
        aInt = m_Int;
        m_Int = _int;

        uint _uint = aUInt;
        aUInt = m_UInt;
        m_UInt = _uint;
        
        long _long = aLong;
        aLong = m_Long;
        m_Long = _long;

        ulong _ulong = aULong;
        aULong = m_ULong;
        m_ULong = _ulong;
        
        float _f = aFloat;
        aFloat = m_Float;
        m_Float = _f;

        double _d = aDouble;
        aDouble = m_Double;
        m_Double = _d;
        
        char _char = aChar;
        aChar = m_Char;
        m_Char = _char;

        string _string = aString;
        aString = m_String;
        m_String = _string;

        Type _type = aType;
        aType = m_Type;
        m_Type = _type;

        Any _any = aAny;
        aAny = m_Any;
        m_Any = _any;

        Enum2 _enum2 = aEnum;
        aEnum = m_Enum2;
        m_Enum2 = _enum2;

        Struct1 _struct1 = aStruct;
        aStruct = m_Struct1;
        m_Struct1 = _struct1;
        
        object _obj = aXInterface;
        aXInterface = m_XInterface;
        m_XInterface = _obj;

        ucss.lang.XComponent _xcomp = aXComponent;
        aXComponent = m_XComponent;
        m_XComponent = _xcomp;

        bool[] _seq = seqBool;
        seqBool = m_seqBool;
        m_seqBool = _seq;
    }

    public bool retBoolean()
    {
        return m_Bool;
    }
    
    public byte retByte()
    {
        return m_Byte;
    }

    public short retShort()
    {
        return m_Short;
    }
    
    public ushort retUShort()
    {
        return m_UShort;
    }
    
    public int retLong()
    {
        return m_Int;
    }
    
    public uint retULong()
    {
        return m_UInt;
    }
    
    public long retHyper()
    {
        return m_Long;
    }

    public ulong retUHyper()
    {
        return m_ULong;
    }

    public float retFloat()
    {
        return m_Float;
    }
    
    public double retDouble()
    {
        return m_Double;
    }
    
    public char retChar()
    {
        return m_Char;
    }
    
    public string retString()
    {
        return m_String;
    }
    
    public Type retType()
    {
        return m_Type;
    }
    
    public uno.Any retAny()
    {
        return m_Any;
    }

    public Enum2 retEnum()
    {
        return m_Enum2;
    }
    public Struct1 retStruct1()
    {
        return m_Struct1;
    }
    
    public object retXInterface()
    {
        return m_XInterface;
    }

    public unoidl.com.sun.star.lang.XComponent retXComponent()
    {
        return m_XComponent;
    }

    public bool[] retSeqBool()
    {
        return m_seqBool;
    }

    public bool attrBoolean
    {
        get {
            return m_Bool;
        }
        set {
            m_Bool = value;
        }
    }

    public byte attrByte
    {
        get {
            return m_Byte;
        }
        set {
            m_Byte = value;
        }
    }

    public short attrShort
    {
        get {
            return m_Short;
        }
        set {
            m_Short = value;
        }
    }

    public ushort attrUShort
    {
        get {
            return m_UShort;
        }
        set {
            m_UShort = value;
        }
    }

    public int attrLong
    {
        get {
            return m_Int;
        }
        set {
            m_Int = value;
        }
    }

    public uint attrULong
    {
        get {
            return m_UInt;
        }
        set {
            m_UInt = value;
        }
    }

    public long attrHyper
    {
        get {
            return m_Long;
        }
        set {
            m_Long = value;
        }
    }

    public ulong attrUHyper
    {
        get {
            return m_ULong;
        }
        set {
            m_ULong = value;
        }
    }

    public float attrFloat
    {
        get {
            return m_Float;
        }
        set {
            m_Float = value;
        }
    }

    public double attrDouble
    {
        get {
            return m_Double;
        }
        set {
            m_Double = value;
        }
    }

    public char attrChar
    {
        get {
            return m_Char;
        }
        set {
            m_Char = value;
        }
    }

    public string attrString
    {
        get {
            return m_String;
        }
        set {
            m_String = value;
        }
    }

    public Type attrType
    {
        get {
            return m_Type;
        }
        set {
            m_Type = value;
        }
    }

    public Any attrAny
    {
        get {
            return m_Any;
        }
        set {
            m_Any = value;
        }
    }

    public Enum2 attrEnum2
    {
        get {
            return m_Enum2;
        }
        set {
            m_Enum2 = value;
        }
    }

    public Struct1 attrStruct1
    {
        get {
            return m_Struct1;
        }
        set {
            m_Struct1 = value;
        }
    }

    public object attrXInterface
    {
        get {
            return m_XInterface;
        }
        set {
            m_XInterface = value;
        }
    }

    public ucss.lang.XComponent attrXComponent
    {
        get {
            return m_XComponent;
        }
        set {
            m_XComponent = value;
        }
    }

    public bool[] attrSeqBoolean
    {
        get {
            return m_seqBool;
        }
        set {
            m_seqBool = value;
        }
    }

    



    Any[] m_args;
    int m_A1;
    int m_A2;
    int m_A3;
    int m_A4;

    bool m_Bool;
    byte m_Byte;
    short m_Short;
    ushort m_UShort;
    int m_Int;
    uint m_UInt;
    long m_Long;
    ulong m_ULong;
    float m_Float;
    double m_Double;
    char m_Char;
    string m_String;
    Type m_Type;
    Any m_Any;
    Enum2 m_Enum2;
    Struct1 m_Struct1;
    object m_XInterface;
    unoidl.com.sun.star.lang.XComponent m_XComponent;
    bool[] m_seqBool;
    
}


