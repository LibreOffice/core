using System;
using System.Collections;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

enum UnoTypeClass {
    Unknown,
    Void,
    Char,
    Boolean,
    Byte,
    Short,
    UnsignedShort,
    Long,
    UnsignedLong,
    Hyper,
    UnsignedHyper,
    Float,
    Double,
    String,
    Type,
    Any,
    Enum,
    Typedef,
    Struct,
    Exception,
    Sequence,
    Interface,
    InterfaceAttribute,
    InterfaceMethod,
    Constant,
    Constants,
    Service,
    Singleton,
    Module
}

struct Constants
{
    public static readonly string UnoVoid = "void";
    public static readonly string UnoType = "type";
    public static readonly string UnoAny =  "any";
    public static readonly string UnoBool = "boolean";
    public static readonly string UnoByte = "byte";
    public static readonly string UnoChar = "char";
    public static readonly string UnoShort = "short";
    public static readonly string UnoUShort = "unsigned short";
    public static readonly string UnoLong = "long";
    public static readonly string UnoULong = "unsigned long";
    public static readonly string UnoHyper = "hyper";
    public static readonly string UnoUHyper = "unsigned hyper";
    public static readonly string UnoString = "string";
    public static readonly string UnoFloat = "float";
    public static readonly string UnoDouble = "double";
    public static readonly string UnoXInterface = "com.sun.star.uno.XInterface";
    public static readonly string Brackets = "[]";

    public static readonly string Object = "System.Object";
    public static readonly string Type = "System.Type";
    public static readonly string Unoidl = "unoidl.";
    public static readonly string Void = "System.Void";
    public static readonly string Any = "uno.Any";
    public static readonly string Boolean = "System.Boolean";
    public static readonly string Char = "System.Char";
    public static readonly string Byte = "System.Byte";
    public static readonly string Int16 = "System.Int16";
    public static readonly string UInt16 = "System.UInt16";
    public static readonly string Int32 = "System.Int32";
    public static readonly string UInt32 = "System.UInt32";
    public static readonly string Int64 = "System.Int64";
    public static readonly string UInt64 = "System.UInt64";
    public static readonly string String = "System.String";
    public static readonly string Single = "System.Single";
    public static readonly string Double = "System.Double";
    public static readonly string Comma = ",";
}

class TypeEmitter : IDisposable
{
    ResolveEventHandler mTypeResolver;
    ModuleBuilder mModuleBuilder;
    ArrayList mExtraAssemblies;
    Hashtable mIncompleteIFaces;
    Hashtable mIncompleteServices;
    Hashtable mIncompleteSingletons;
    Hashtable mGeneratedStructs;
    Config mConfig;

    Type mTypeException = null;
    Type mTypeRuntimeException = null;

    readonly static MethodAttributes cCtorMethodAttr =
    MethodAttributes.Public |
    MethodAttributes.HideBySig |
    MethodAttributes.SpecialName |
    MethodAttributes.RTSpecialName;
    /* | xxx todo: ??? compiler does not know Instance ???
       ::System::Reflection::MethodAttributes::Instance*/

    MethodInfo mMethodInfoTypeGetTypeFromHandle;

    class IFaceEntry
    {
        public UnoXInterfaceTypeDescription mType;
        public TypeBuilder mTypeBuilder;
    }

    class ServiceEntry
    {
        public UnoXServiceTypeDescription mType;
        public TypeBuilder mTypeBuilder;
    }

    class SingletonEntry
    {
        public UnoXSingletonTypeDescription mType;
        public TypeBuilder mTypeBuilder;
    }

    class StructEntry
    {
        public string[] mMemberNames;
        public Type[] mParamTypes;
        public ConstructorInfo mDefaultConstructor;
        public ConstructorInfo mConstructor;
    }

    public TypeEmitter (Config config, ModuleBuilder builder)
    {
        mConfig = config;

        // load extra assemblies
        mExtraAssemblies = new ArrayList ();
        foreach (string assemblyPath in mConfig.mExtraAssemblies)
            mExtraAssemblies.Add (Assembly.LoadFrom (assemblyPath));

        mTypeResolver = new ResolveEventHandler (TypeResolveHandler);
        mModuleBuilder = builder;

        mIncompleteIFaces = new Hashtable ();
        mIncompleteServices = new Hashtable ();
        mIncompleteSingletons = new Hashtable ();
        mGeneratedStructs = new Hashtable ();

        Type[] paramTypes = { typeof (RuntimeTypeHandle) };
        mMethodInfoTypeGetTypeFromHandle = typeof (Type).GetMethod ("GetTypeFromHandle", paramTypes);
    }

    public ResolveEventHandler ResolveEventHandler
    {
    get
    {
        return mTypeResolver;
    }
    }

    Assembly TypeResolveHandler (object o, ResolveEventArgs args)
    {
        Type ret = mModuleBuilder.GetType (args.Name, false);
        //Console.WriteLine ("mModuleBuilder.GetType yields {0}", ret);

#if __MonoCS__
        if (ret is TypeBuilder) {
            TypeBuilder tb = ret as TypeBuilder;
            //Console.WriteLine ("{0} is type builder", tb);
            if (tb.IsCreated ()) {
                ret = tb.CreateType ();
                //Console.WriteLine ("resolving to created {0} {1}", ret, tb);
            }
        }
#endif
        if (ret == null) {
            IFaceEntry entry = mIncompleteIFaces [args.Name] as IFaceEntry;
            if (entry != null)
                ret = entry.mTypeBuilder;
        }

        if (ret == null && mExtraAssemblies != null) {
            //Console.WriteLine ("assemblies {0}", mExtraAssemblies);
            foreach (Assembly assembly in mExtraAssemblies) {
                ret = assembly.GetType (args.Name, false);
                if (ret != null) {
                    if (mConfig.mVerbose) {
                        Console.WriteLine ("> resolving type {0} from {1}.",
                                           args.Name, ret.Assembly.FullName);
				    }
                    break;
                }
            }
            //Console.WriteLine ("done {0}", ret);
        }

        if (ret != null)
            return ret.Assembly;

        return null;
    }

    Type GetType (string name, bool throwExc)
    {
        Type ret = mModuleBuilder.GetType (name, false);
        //Console.WriteLine ("mModuleBuilder.GetType yields {0}", ret);

#if __MonoCS__
        if (ret is TypeBuilder) {
            TypeBuilder tb = ret as TypeBuilder;
            //Console.WriteLine ("{0} is type builder", tb);
            if (tb.IsCreated ()) {
                ret = tb.CreateType ();
                //Console.WriteLine ("resolving to created {0} {1}", ret, tb);
            }
        }
#endif
        if (ret == null) {
            //Console.WriteLine ("looking name {0}", name);
            IFaceEntry entry = mIncompleteIFaces [name] as IFaceEntry;
            if (entry != null)
                ret = entry.mTypeBuilder;
        }

        //try the cli_basetypes assembly
        if (ret == null) {
            ret = Type.GetType (name + ",cli_basetypes");
        }

        if (ret == null) {
            try  {
                // may call on type_resolve()
                return Type.GetType (name, throwExc);
            } catch (Exception e) {
                //If the type is not found one may have forgotten to specify assemblies with
                //additional types
                if (throwExc)
                    throw new Exception ("\nThe type " + name + " \n could not be found. Did you forget to " +
                                         "specify an additional assembly with the --reference option?\n", e);
            }
        }

        return ret;
    }

    public Type GetType (UnoXEnumTypeDescription xtd)
    {
    //Console.WriteLine ("emit enum {0} {1}", xtd.Name, xtd.Length);
    string name = "unoidl." + xtd.Name;

    Type ret = GetType (name, false /* no exc */);
    if (ret == null) {
//         Emit::EnumBuilder * enum_builder =
//             m_module_builder->DefineEnum(
//                 cts_name,
//                 (TypeAttributes) (TypeAttributes::Public |
// //                                   TypeAttributes::Sealed |
//                                   TypeAttributes::AnsiClass),
//                 __typeof (::System::Int32) );
        // workaround enum builder bug
        TypeBuilder enumBuilder =
        mModuleBuilder.DefineType (name,
                       TypeAttributes.Public |
                       TypeAttributes.Sealed,
                       typeof (System.Enum));
        enumBuilder.DefineField ("value__", typeof (System.Int32),
                     FieldAttributes.Public |
                     FieldAttributes.SpecialName |
                     FieldAttributes.RTSpecialName);

        int length = xtd.Length;
        for (int pos = 0; pos < length; pos ++) {
//             enum_builder->DefineLiteral(
//                 ustring_to_String( enum_names[ enum_pos ] ),
//                 __box ((::System::Int32) enum_values[ enum_pos ]) );
        FieldBuilder fieldBuilder =
            enumBuilder.DefineField (xtd.ValueName (pos),
                         enumBuilder,
                         FieldAttributes.Public |
                         FieldAttributes.Static |
                         FieldAttributes.Literal);
        fieldBuilder.SetConstant (xtd.Value (pos));
        }

        if (mConfig.mVerbose)
             Console.WriteLine ("> emitting enum type {0}", name );

        ret = enumBuilder.CreateType ();
    }

    return ret;
    }

    public Type GetType (UnoXInterfaceTypeDescription xtd)
    {
        //Console.WriteLine ("get iface {0}", xtd.Name);

        if (String.Compare (xtd.Name, "com.sun.star.uno.XInterface") == 0) {
            return typeof (object);
    }

    string name = "unoidl." + xtd.Name;

    Type ret = GetType (name, false /* no exc */);

    if (ret == null) {
        //Console.WriteLine ("creating name {0}", name);
        TypeBuilder typeBuilder;

        TypeAttributes attr = TypeAttributes.Public |
        TypeAttributes.Interface |
        TypeAttributes.Abstract |
        TypeAttributes.AnsiClass;

        int length = xtd.BaseTypes;
        if (length > 0) {
        ArrayList ifaces = new ArrayList ();
        int i;

        for (i = 0; i < length; i ++) {
            UnoXInterfaceTypeDescription baseType = xtd.BaseType (i);
            if (String.Compare (baseType.Name, "com.sun.star.uno.XInterface") != 0)
            ifaces.Add (baseType);
        }
        Type[] baseInterfaces = new Type [ifaces.Count];

        i = 0;
        foreach (UnoXInterfaceTypeDescription iface in ifaces) {
            baseInterfaces[i] = GetType (iface);
            i++;
        }

        typeBuilder = mModuleBuilder.DefineType (name, attr, null, baseInterfaces);
        } else {
            System.Console.WriteLine ("warning: IDL interface {0} is not derived from " +
                                      "com.sun.star.uno.XInterface!", name);

            typeBuilder = mModuleBuilder.DefineType (name, attr);
        }

        // insert to be completed
        IFaceEntry entry = new IFaceEntry ();
        entry.mType = xtd;
        entry.mTypeBuilder = typeBuilder;
        mIncompleteIFaces [name] = entry;

        // type is incomplete
	    ret = typeBuilder;
    }

    return ret;
    }

    string PolymorphicStructNameToStructName (string name)
    {
    if (!name.EndsWith (">"))
        return name;

    int index = name.IndexOf ('<');

    if (index == -1)
        return name;

    return name.Substring (0, index);
    }

    Type TypeException ()
    {
        if (mTypeException == null) {
            mTypeException = GetType ("unoidl.com.sun.star.uno.Exception", false);

            if (mTypeException == null) {
                // define hardcoded type unoidl.com.sun.star.uno.Exception
                TypeBuilder typeBuilder = mModuleBuilder.DefineType ("unoidl.com.sun.star.uno.Exception",
                                                                     TypeAttributes.Public |
                                                                     TypeAttributes.BeforeFieldInit |
                                                                     TypeAttributes.AnsiClass,
                                                                     typeof (System.Exception));
                FieldBuilder fieldContext = typeBuilder.DefineField ("Context", typeof (object),
                                                                     FieldAttributes.Public);
                // default .ctor
                typeBuilder.DefineDefaultConstructor (cCtorMethodAttr);
                // .ctor
                Type[] paramTypes = new Type [2];
                paramTypes [0] = typeof (string);
                paramTypes [1] = typeof (object);
                ConstructorBuilder ctorBuilder = typeBuilder.DefineConstructor (cCtorMethodAttr,
                                                                                CallingConventions.Standard,
                                                                                paramTypes);
                ctorBuilder.DefineParameter (1, ParameterAttributes.In, "Message");
                ctorBuilder.DefineParameter (2, ParameterAttributes.In, "Context");
                ILGenerator code = ctorBuilder.GetILGenerator ();
                code.Emit (OpCodes.Ldarg_0);
                code.Emit (OpCodes.Ldarg_1);
                paramTypes = new Type [1];
                paramTypes [0] = typeof (string);
                code.Emit (OpCodes.Call, typeof (System.Exception).GetConstructor (paramTypes));
                code.Emit( OpCodes.Ldarg_0 );
                code.Emit( OpCodes.Ldarg_2 );
                code.Emit( OpCodes.Stfld, fieldContext );
                code.Emit( OpCodes.Ret );

                if (mConfig.mVerbose)
                    Console.WriteLine ("> emitting exception type unoidl.com.sun.star.uno.Exception");

                mTypeException = typeBuilder.CreateType ();
            }
        }

        return mTypeException;
    }

    Type TypeRuntimeException ()
    {
        if (mTypeRuntimeException == null) {
            mTypeRuntimeException = GetType ("unoidl.com.sun.star.uno.RuntimeException", false);
            if (mTypeRuntimeException == null) {
                // define hardcoded type unoidl.com.sun.star.uno.RuntimeException
                Type typeException = TypeException ();
                TypeBuilder typeBuilder = mModuleBuilder.DefineType ("unoidl.com.sun.star.uno.RuntimeException",
                                                                     TypeAttributes.Public |
                                                                     TypeAttributes.BeforeFieldInit |
                                                                     TypeAttributes.AnsiClass,
                                                                     typeException);
                // default .ctor
                typeBuilder.DefineDefaultConstructor (cCtorMethodAttr);
                // .ctor
                Type[] paramTypes = new Type [2];
                paramTypes [0] = typeof (string);
                paramTypes [1] = typeof (object);
                ConstructorBuilder ctorBuilder = typeBuilder.DefineConstructor (cCtorMethodAttr,
                                                                                CallingConventions.Standard,
                                                                                paramTypes);
                ctorBuilder.DefineParameter (1, ParameterAttributes.In, "Message");
                ctorBuilder.DefineParameter (2, ParameterAttributes.In, "Context");
                ILGenerator code = ctorBuilder.GetILGenerator ();
                code.Emit (OpCodes.Ldarg_0);
                code.Emit (OpCodes.Ldarg_1);
                code.Emit (OpCodes.Ldarg_2 );
                code.Emit (OpCodes.Call,
                           typeException.GetConstructor (paramTypes));
                code.Emit (OpCodes.Ret);

                if (mConfig.mVerbose)
                    Console.WriteLine ("> emitting exception type unoidl.com.sun.star.uno.RuntimeException");

                mTypeRuntimeException = typeBuilder.CreateType ();
            }
        }

        return mTypeRuntimeException;
    }

    public Type GetType (UnoXServiceTypeDescription xtd)
    {
        // Console.WriteLine ("get service {0}", xtd.Name);

        if (!xtd.IsSingleInterfaceBased)
            return null;

        string name = "unoidl." + xtd.Name;

        Type ret = GetType (name, false /* no exc */);

        if (ret != null)
            return ret;

        TypeAttributes attr =
            TypeAttributes.Public |
            TypeAttributes.Sealed |
            TypeAttributes.BeforeFieldInit |
            TypeAttributes.AnsiClass;

        // insert to be completed
        ServiceEntry entry = new ServiceEntry ();
        entry.mType = xtd;
        entry.mTypeBuilder = mModuleBuilder.DefineType (name, attr);
        mIncompleteServices.Add (name, entry);

        return entry.mTypeBuilder;
    }

    static void EmitLdarg (ILGenerator code, int index)
    {
        switch (index) {
        case 0:
            code.Emit( OpCodes.Ldarg_0 );
            break;
        case 1:
            code.Emit( OpCodes.Ldarg_1 );
            break;
        case 2:
            code.Emit( OpCodes.Ldarg_2 );
            break;
        case 3:
            code.Emit( OpCodes.Ldarg_3 );
            break;
        default:
            if (index < 0x100)
                code.Emit (OpCodes.Ldarg_S, (byte) index);
            else if (index < 0x8000)
                code.Emit (OpCodes.Ldarg_S, (System.Int16) index);
            else
                code.Emit (OpCodes.Ldarg, index);
            break;
        }
    }

    /** For example, there is a uno type
        com.sun.star.Foo<char, long>.
        The values in the type list
        are uno types and are replaced by cli types, such as System.Char,
        System.Int32, etc.

        Strings can be as complicated as this
        test.MyStruct<char,test.MyStruct<long, []string>>
    */
    string MapUnoPolymorphicName (string unoName)
    {
        int index = unoName.IndexOf('<');
        if (index == -1)
            return unoName;

        System.Text.StringBuilder builder = new System.Text.StringBuilder (unoName.Substring (0, index + 1));

        //Find the first occurrence of ','
        //If the parameter is a polymorphic struct then we neede to ignore everything
        //between the brackets because it can also contain commas
        //get the type list within < and >
        int endIndex = unoName.Length - 1;
        index ++;
        int cur = index;
        int countParams = 0;
        while (cur <= endIndex) {
            char c = unoName [cur];

            if (c == ',' || c == '>') {
                //insert a comma if needed
                if (countParams != 0)
                    builder.Append (",");
                countParams++;
                string param = unoName.Substring (index, cur - index);
                //skip the comma
                cur ++;
                //the the index to the beginning of the next param
                index = cur;
                builder.Append (MapUnoTypeName (param));
            } else if (c == '<') {
                cur++;
                //continue until the matching '>'
                int numNested = 0;
                for (;; cur ++) {
                    char curChar = unoName [cur];
                    if (curChar == '<')
                        numNested ++;
                    else if (curChar == '>') {
                        if (numNested > 0)
                            numNested --;
                        else
                            break;
                    }
                }
            }
            cur ++;
        }

        builder.Append ('>');
        return builder.ToString();
    }

    string MapUnoTypeName (string typeName)
    {
        System.Text.StringBuilder buf= new System.Text.StringBuilder ();
        string unoName = String.Copy (typeName);

        //determine if the type is a sequence and its dimensions
        int dims = 0;
        if (typeName.StartsWith ("[")) {
            int index= 1;
            while (true) {
                if (typeName [index ++] == ']')
                    dims++;
                if (typeName [index ++] != '[')
                    break;
            }
            unoName = unoName.Substring (index - 1);
        }

        if (unoName.Equals (Constants.UnoBool))
            buf.Append (Constants.Boolean);
        else if (unoName.Equals(Constants.UnoChar))
            buf.Append (Constants.Char);
        else if (unoName.Equals(Constants.UnoByte))
            buf.Append (Constants.Byte);
        else if (unoName.Equals(Constants.UnoShort))
            buf.Append (Constants.Int16);
        else if (unoName.Equals(Constants.UnoUShort))
            buf.Append (Constants.UInt16);
        else if (unoName.Equals(Constants.UnoLong))
            buf.Append (Constants.Int32);
        else if (unoName.Equals(Constants.UnoULong))
            buf.Append (Constants.UInt32);
        else if (unoName.Equals(Constants.UnoHyper))
            buf.Append (Constants.Int64);
        else if (unoName.Equals(Constants.UnoUHyper))
            buf.Append (Constants.UInt64);
        else if (unoName.Equals(Constants.UnoFloat))
            buf.Append (Constants.Single);
        else if (unoName.Equals(Constants.UnoDouble))
            buf.Append (Constants.Double);
        else if (unoName.Equals(Constants.UnoString))
            buf.Append (Constants.String);
        else if (unoName.Equals(Constants.UnoVoid))
            buf.Append (Constants.Void);
        else if (unoName.Equals(Constants.UnoType))
            buf.Append (Constants.Type);
        else if (unoName.Equals(Constants.UnoXInterface))
            buf.Append (Constants.Object);
        else if (unoName.Equals(Constants.UnoAny)) {
            buf.Append (Constants.Any);
        } else {
            //put "unoidl." at the beginning
            buf.Append (Constants.Unoidl);
            buf.Append (MapUnoPolymorphicName (unoName));
        }

        // apend []
        for (;dims-- > 0;)
            buf.Append (Constants.Brackets);

        // Console.WriteLine ("MapUnoTypeName {0} => {1}", typeName, buf.ToString ());

        return buf.ToString();
    }

    public Type GetType (UnoXCompoundTypeDescription xtd)
    {
        // Console.WriteLine ("get compound type {0}", xtd.Name);

        if (xtd.TypeClass == UnoTypeClass.Exception) {
            if (xtd.Name.Equals ("com.sun.star.uno.Exception"))
                return TypeException ();

            if (xtd.Name.Equals ("com.sun.star.uno.RuntimeException"))
                return TypeRuntimeException ();
        }

        string name = "unoidl." + xtd.Name;

        // if the struct is an instantiated polymorpic struct then we create the simple struct name
        // For example:
        //   void func ([in] PolyStruct<boolean> arg);
        //   PolyStruct<boolean> will be converted to PolyStruct
        name = PolymorphicStructNameToStructName (name);

        Type ret = GetType (name, false /* no exc */);
        UnoXStructTypeDescription xsd = xtd as UnoXStructTypeDescription;

        if (ret == null) {
            // Console.WriteLine ("create compound type {0}", name);
            UnoXTypeDescription baseTD = xtd.BaseType;
            Type baseType = baseTD != null ? GetType (baseTD) : typeof (object);
            CustomAttributeBuilder attrBuilder;
            TypeBuilder typeBuilder = mModuleBuilder.DefineType (name,
                                                                 TypeAttributes.Public |
                                                                 TypeAttributes.BeforeFieldInit |
                                                                 TypeAttributes.AnsiClass,
                                                                 baseType);
            int i;

            // Polymorphic struct, define uno.TypeParametersAttribute
            // A polymorphic struct cannot have a basetype.
            // When we create the template of the struct then we have no exact types
            // and the name does not contain a parameter list
            if (xsd != null && xsd.TypeParameters > 0) {
                object[] args = new object [xsd.TypeParameters];

                for (i = 0; i < xsd.TypeParameters; i ++)
                    args [i] = xsd.TypeParameter (i);
                object[] aargs = { args };

                Type[] typesCtor = { typeof (string[]) };
                attrBuilder = new CustomAttributeBuilder (typeof (uno.TypeParametersAttribute).GetConstructor (typesCtor), aargs);
                typeBuilder.SetCustomAttribute (attrBuilder);
            }

            // optional: lookup base type whether generated entry of this session
            StructEntry baseTypeEntry = null;
            if (baseType != null)
                baseTypeEntry = (StructEntry) mGeneratedStructs [baseType.FullName];

            // members
            for (i = 0; i < xtd.MemberTypes; i ++) {
                if (xtd.MemberType (i) == null)
                    throw new Exception ("Missing type description . Check if you need to " +
                                         "specify additional RDBs with the --extra option. Type missing for: " +
                                         xtd.Name + "::" + xtd.MemberName (i));
            }

            // collect base types; wrong order
            ArrayList baseTypes = new ArrayList (3);
            int allMembersLength = 0;
            for (Type baseTypePos = baseType; !baseTypePos.Equals (typeof (object)); baseTypePos = baseTypePos.BaseType) {
                baseTypes.Add (baseTypePos);
                if (baseTypePos.Equals (typeof (System.Exception))) {
                    // special Message member
                    allMembersLength ++;
                    break; // don't include System.Exception base classes
                } else {
                    allMembersLength += baseTypePos.GetFields (BindingFlags.Instance |
                                                               BindingFlags.Public |
                                                               BindingFlags.DeclaredOnly).Length;
                }
            }

            // create all_members arrays; right order
            string[] allMemberNames = new string [allMembersLength + xtd.MemberTypes];
            Type[] allParamTypes = new Type [allMembersLength + xtd.MemberTypes];
            int memberPos = 0;
            for (i = baseTypes.Count - 1; i >= 0; i--) {
                Type type = baseTypes [i] as Type;

                if (type.Equals (typeof (System.Exception))) {
                    allMemberNames [memberPos] = "Message";
                    allParamTypes [memberPos] = typeof (string);
                    memberPos ++;
                } else {
                    StructEntry baseEntry = mGeneratedStructs [type.FullName] as StructEntry;
                    if (baseEntry == null) {
                        // complete type
                        FieldInfo[] fields = type.GetFields (BindingFlags.Instance |
                                                             BindingFlags.Public |
                                                             BindingFlags.DeclaredOnly);
                        foreach (FieldInfo fieldInfo in fields) {
                            allMemberNames [memberPos] = fieldInfo.Name;
                            allParamTypes [memberPos] = fieldInfo.FieldType;
                            memberPos ++;
                        }
                    } else {
                        // generated during this session:
                        // members may be incomplete ifaces
                        int j;
                        for (j = 0; j < baseEntry.mParamTypes.Length; j ++) {
                            allMemberNames [memberPos] = baseEntry.mMemberNames [j];
                            allParamTypes [memberPos] = baseEntry.mParamTypes [j];
                            memberPos ++;
                        }
                    }
                }
            }
            if (allMembersLength != memberPos)
                throw new Exception ("something went wrong, allMembersLength != memberPos");


            // build up entry
            StructEntry entry = new StructEntry ();
            entry.mMemberNames = new string [xtd.MemberTypes];
            entry.mParamTypes = new Type [xtd.MemberTypes];

            // add members
            FieldBuilder[] members = new FieldBuilder [xtd.MemberTypes];
            int curParamIndex = 0;
            int typeParamPos = 0;

            for (memberPos = 0; memberPos < xtd.MemberTypes; memberPos ++) {
                string fieldName = xtd.MemberName (memberPos);
                Type fieldType;

                //Special handling of struct parameter types
                bool parameterizedType = false;
                if (xtd.MemberType (memberPos).TypeClass == UnoTypeClass.Unknown) {
                    parameterizedType = true;
                    if (xsd != null && typeParamPos < xsd.TypeParameters) {
                        fieldType = typeof (object);
                        typeParamPos ++;
                    } else {
                        throw new Exception ("unexpected member type in " + xtd.Name);
                    }
                } else {
                    fieldType = GetType (xtd.MemberType (memberPos));
                }
                members [memberPos] = typeBuilder.DefineField (fieldName, fieldType, FieldAttributes.Public);

                //parameterized type (polymorphic struct) ?
                if (parameterizedType && xsd != null) {
                    object[] args = { xsd.TypeParameter (curParamIndex ++) };
                    Type[] ctorTypes = { typeof (string) };
                    attrBuilder = new CustomAttributeBuilder (typeof (uno.ParameterizedTypeAttribute).GetConstructor (ctorTypes), args);
                    members [memberPos].SetCustomAttribute (attrBuilder);
                }

                // add to all_members
                allMemberNames [allMembersLength + memberPos] = fieldName;
                allParamTypes [allMembersLength + memberPos] = fieldType;

                // add to entry
                entry.mMemberNames [memberPos] = fieldName;
                entry.mParamTypes [memberPos] = fieldType;
            }

            allMembersLength += xtd.MemberTypes;

            // default .ctor
            ConstructorBuilder ctorBuilder = typeBuilder.DefineConstructor (cCtorMethodAttr, CallingConventions.Standard, new Type [0]);
            ILGenerator code = ctorBuilder.GetILGenerator ();

            code.Emit (OpCodes.Ldarg_0);
            // Console.WriteLine ("baseType: {0}", baseType);
            code.Emit (OpCodes.Call, baseTypeEntry == null ? baseType.GetConstructor (new Type [0]) : baseTypeEntry.mDefaultConstructor);

            // default initialize members
            for (memberPos = 0; memberPos < xtd.MemberTypes; memberPos ++) {
                FieldInfo fieldInfo = members [memberPos];

                // default initialize
                // string, type, enum, sequence, struct, exception, any
                if (fieldInfo.FieldType.Equals (typeof (string))) {
                    code.Emit (OpCodes.Ldarg_0);
                    code.Emit (OpCodes.Ldstr, "");
                    code.Emit (OpCodes.Stfld, fieldInfo);
                } else if (fieldInfo.FieldType.Equals (typeof (Type))) {
                    code.Emit (OpCodes.Ldarg_0);
                    code.Emit (OpCodes.Ldtoken, typeof (void));
                    code.Emit (OpCodes.Call, mMethodInfoTypeGetTypeFromHandle);
                    code.Emit (OpCodes.Stfld, fieldInfo);
                } else if (fieldInfo.FieldType.IsArray) {
                    code.Emit (OpCodes.Ldarg_0);
                    code.Emit (OpCodes.Ldc_I4_0);
                    code.Emit (OpCodes.Newarr, fieldInfo.FieldType.GetElementType ());
                    code.Emit (OpCodes.Stfld, fieldInfo);
                } else if (fieldInfo.FieldType.IsValueType) {
                    if (fieldInfo.FieldType.FullName.Equals ("uno.Any")) {
                        code.Emit (OpCodes.Ldarg_0);
                        code.Emit (OpCodes.Ldsfld, typeof (uno.Any).GetField ("VOID"));
                        code.Emit (OpCodes.Stfld, fieldInfo);
                    }
                } else if (fieldInfo.FieldType.IsClass) {
                    // may be XInterface
                    if (!fieldInfo.FieldType.Equals (typeof (object))) {
                        code.Emit (OpCodes.Ldarg_0);
                        code.Emit (OpCodes.Newobj, fieldInfo.FieldType.GetConstructor (new Type [0]));
                        code.Emit (OpCodes.Stfld, fieldInfo);
                    }
                }
            }

            code.Emit (OpCodes.Ret);
            entry.mDefaultConstructor = ctorBuilder;

            // parameterized .ctor including all base members
            ctorBuilder = typeBuilder.DefineConstructor (cCtorMethodAttr, CallingConventions.Standard, allParamTypes);
            for (memberPos = 0; memberPos < allMembersLength; memberPos ++)
                ctorBuilder.DefineParameter (memberPos + 1, ParameterAttributes.In, allMemberNames [memberPos]);

            code = ctorBuilder.GetILGenerator ();
            // call base .ctor
            code.Emit (OpCodes.Ldarg_0); // push this
            int baseMembersLength = allMembersLength - xtd.MemberTypes;
            Type[] paramTypes = new Type [baseMembersLength];

            for (memberPos = 0; memberPos < baseMembersLength; memberPos ++) {
                EmitLdarg (code, memberPos + 1);
                paramTypes [memberPos] = allParamTypes [memberPos];
            }

            code.Emit (OpCodes.Call, baseTypeEntry == null ? baseType.GetConstructor (paramTypes) : baseTypeEntry.mConstructor);

            // initialize members

            for (memberPos = 0; memberPos < xtd.MemberTypes; memberPos ++) {
                code.Emit (OpCodes.Ldarg_0); // push this
                EmitLdarg (code, memberPos + baseMembersLength + 1);
                code.Emit (OpCodes.Stfld, members [memberPos]);
            }

            code.Emit (OpCodes.Ret);
            entry.mConstructor = ctorBuilder;

            if (mConfig.mVerbose)
                Console.WriteLine ("> emitting {0} type {1}", xtd is UnoXStructTypeDescription ? "struct" : "exception", name);

            // new entry
            mGeneratedStructs.Add (name, entry);
            //Console.WriteLine ("added entry to mGeneratedStructs: {0}", name);
            //if (baseTD != null)
            //Console.WriteLine ("baseTD: {0}", baseTD.Name, GetType (baseTD).Name);
            ret = typeBuilder.CreateType ();
        }

        // This is neaded
        // for constructing the service code. We can only do that if the
        // struct is completed.
        StructEntry anEntry = mGeneratedStructs [name] as StructEntry;
        if ( anEntry != null && xsd != null && xsd.TypeArguments > 0) {
            //Console.WriteLine ("polymorphic struct: call uno.PolymorphicType.GetType ({0}, {1})", ret, xtd.Name);
            ret = uno.PolymorphicType.GetType (ret, MapUnoTypeName (xtd.Name));
            //Console.WriteLine ("polymorphic struct: {0} ({1})", ret, xtd.Name);
        }

        return ret;
    }

    public Type GetType (UnoXConstantTypeDescription xtd)
    {
        //Console.WriteLine ("get constant type {0}", xtd.Name);

        string name = "unoidl." + xtd.Name;

        Type ret = GetType (name, false /* no exc */);

        if (ret == null) {
            object constant = xtd.ConstantValue;

            TypeBuilder typeBuilder = mModuleBuilder.DefineType (name,
                                                                 TypeAttributes.Public |
                                                                 TypeAttributes.Sealed |
                                                                 TypeAttributes.BeforeFieldInit |
                                                                 TypeAttributes.AnsiClass);
            FieldBuilder field = typeBuilder.DefineField (name.Substring (name.LastIndexOf ('.') + 1),
                                                          constant.GetType (),
                                                          FieldAttributes.Public |
                                                          FieldAttributes.Static |
                                                          FieldAttributes.Literal);
            field.SetConstant (constant);

            if (mConfig.mVerbose)
                Console.WriteLine ("> emitting constant type {0}", name);

            ret = typeBuilder.CreateType ();
        }

        return ret;
    }

    public Type GetType (UnoXConstantsTypeDescription xtd)
    {
        //Console.WriteLine ("get constants type {0}", xtd.Name);

        string name = "unoidl." + xtd.Name;

        Type ret = GetType (name, false /* no exc */);

        if (ret == null) {
            TypeBuilder typeBuilder = mModuleBuilder.DefineType (name,
                                                                 TypeAttributes.Public |
                                                                 TypeAttributes.Sealed |
                                                                 TypeAttributes.BeforeFieldInit |
                                                                 TypeAttributes.AnsiClass);
            int i;
            for (i = 0; i < xtd.Constants; i ++) {
                UnoXConstantTypeDescription constantTD = xtd.Constant (i);
                object constant = constantTD.ConstantValue;
                FieldBuilder field = typeBuilder.DefineField (constantTD.Name.Substring (constantTD.Name.LastIndexOf ('.') + 1),
                                                              constant.GetType (),
                                                              FieldAttributes.Public |
                                                              FieldAttributes.Static |
                                                              FieldAttributes.Literal);
                field.SetConstant (constant);
            }

            if (mConfig.mVerbose)
                Console.WriteLine ( "> emitting constants group type {0}", name);

            ret = typeBuilder.CreateType ();
        }

        return ret;
    }

    public Type GetType (UnoXSingletonTypeDescription xtd)
    {
        //Console.WriteLine ("get singleton {0}", xtd.Name);

        if (!xtd.IsInterfaceBased)
            return null;

        //Console.WriteLine ("singleton {0} is interface based", xtd.Name);

        string name = "unoidl." + xtd.Name;

        Type ret = GetType (name, false /* no exc */);

        if (ret != null)
            return ret;

        TypeAttributes attr =
            TypeAttributes.Public |
            TypeAttributes.Sealed |
            TypeAttributes.BeforeFieldInit |
            TypeAttributes.AnsiClass;

        // insert to be completed
        SingletonEntry entry = new SingletonEntry ();
        entry.mType = xtd;
        entry.mTypeBuilder = mModuleBuilder.DefineType (name, attr);
        mIncompleteSingletons.Add (name, entry);

        return entry.mTypeBuilder;
    }

    public Type GetType (UnoXTypeDescription xtd)
    {
        UnoTypeClass tc = xtd.TypeClass;

        switch (tc) {
        case UnoTypeClass.Void:
            return typeof (void);
        case UnoTypeClass.Char:
            return typeof (char);
        case UnoTypeClass.Boolean:
            return typeof (bool);
        case UnoTypeClass.Byte:
            return typeof (byte);
        case UnoTypeClass.Short:
            return typeof (Int16);
        case UnoTypeClass.UnsignedShort:
            return typeof (UInt16);
        case UnoTypeClass.Long:
            return typeof (Int32);
        case UnoTypeClass.UnsignedLong:
            return typeof (UInt32);
        case UnoTypeClass.Hyper:
            return typeof (Int64);
        case UnoTypeClass.UnsignedHyper:
            return typeof (UInt64);
        case UnoTypeClass.Float:
            return typeof (Single);
        case UnoTypeClass.Double:
            return typeof (double);
        case UnoTypeClass.String:
            return typeof (string);
        case UnoTypeClass.Type:
            return typeof (Type);
        case UnoTypeClass.Any:
            return typeof (uno.Any);
        case UnoTypeClass.Enum:
            return GetType (xtd as UnoXEnumTypeDescription);
        case UnoTypeClass.Interface:
            return GetType (xtd as UnoXInterfaceTypeDescription);
        case UnoTypeClass.Struct:
        case UnoTypeClass.Exception:
            return GetType (xtd as UnoXCompoundTypeDescription);
        case UnoTypeClass.Module:
            return null;
        case UnoTypeClass.Sequence:
            Type elementType = GetType ((xtd as UnoXIndirectTypeDescription).ReferencedType);
            Type retType = GetType (elementType.FullName + "[]", true);
            uno.PolymorphicType polyType = elementType as uno.PolymorphicType;
            if (polyType != null) {
                string name = polyType.PolymorphicName + "[]";
                retType = uno.PolymorphicType.GetType (retType, name);
            }
            return retType;
        case UnoTypeClass.Typedef:
            return GetType ((xtd as UnoXIndirectTypeDescription).ReferencedType);
        case UnoTypeClass.Constant:
            return GetType (xtd as UnoXConstantTypeDescription);
        case UnoTypeClass.Constants:
            return GetType (xtd as UnoXConstantsTypeDescription);
        case UnoTypeClass.Service:
            return GetType (xtd as UnoXServiceTypeDescription);
        case UnoTypeClass.Singleton:
            return GetType (xtd as UnoXSingletonTypeDescription);
        default:
            // fixme, use double for unfinished types
            //Console.WriteLine ("warning: unfinished type reached: {0}", xtd.Name);
            return typeof (void);
        }

        //throw new Exception (String.Format ("Unknown type requested {0}", tc));
    }

    CustomAttributeBuilder IFaceMethodExceptionAttribute (UnoXInterfaceMethodTypeDescription method)
    {
        Type[] exceptionTypes = new Type [method.ExceptionTypes];
        int i;

        for (i = 0; i < method.ExceptionTypes; i ++) {
            exceptionTypes [i] = GetType (method.ExceptionType (i));
        }

        return ExceptionAttribute (exceptionTypes);
    }

    CustomAttributeBuilder ExceptionAttribute (Type[] exceptionTypes)
    {
        CustomAttributeBuilder attrBuilder = null;

        if (exceptionTypes.Length > 0) {
            object[] args = { exceptionTypes };
            Type[] arTypesCtor = { typeof (Type[]) };
            ConstructorInfo ctorInfo = typeof (uno.ExceptionAttribute).GetConstructor (arTypesCtor);
            attrBuilder = new CustomAttributeBuilder (ctorInfo, args);
        }

        return attrBuilder;
    }

    Type[] GetTypes (UnoXTypeDescription[] tds)
    {
        Type[] types = new Type [tds.Length];
        int i;

        for (i = 0; i < tds.Length; i ++)
            types [i] = GetType (tds [i]);

        return types;
    }

    Type CompleteIFaceType (IFaceEntry entry)
        {
            //Console.WriteLine ("going to complete {0}", entry.mTypeBuilder.FullName);

    // complete base interfaces first
            foreach (Type type in entry.mTypeBuilder.GetInterfaces ()) {
                IFaceEntry baseEntry = mIncompleteIFaces [type.FullName] as IFaceEntry;
                if (baseEntry != null)
                    CompleteIFaceType (baseEntry);
            }

            // emit members
            const MethodAttributes methodAttr =
                MethodAttributes.Public |
                MethodAttributes.Abstract |
                MethodAttributes.Virtual |
                MethodAttributes.NewSlot |
                MethodAttributes.HideBySig;
            int i;

            for (i = 0; i < entry.mType.Members; i ++) {
                UnoXInterfaceMemberTypeDescription member = entry.mType.Member (i);
                MethodBuilder methodBuilder;
                CustomAttributeBuilder attrBuilder;

                if (member.TypeClass == UnoTypeClass.InterfaceMethod) {
                    UnoXInterfaceMethodTypeDescription method = new UnoXInterfaceMethodTypeDescription (member.Handle);
                    UnoXMethodParameter[] parameters = new UnoXMethodParameter [method.Parameters];
                    Type[] pTypes = new Type [method.Parameters];
                    int j;

                    //Console.WriteLine ("method {0}.{1}", entry.mTypeBuilder.FullName, method.MemberName);

                    // first determine all types
                    for (j = 0; j < method.Parameters; j ++) {
                        parameters [j] = method.Parameter (j);
                        pTypes [j] = GetType (parameters [j].Type);
                        if (parameters [j].IsOut)
                            pTypes [j] = GetType (pTypes [j].FullName + "&", true);
                    }

                    // create method
                    methodBuilder = entry.mTypeBuilder.DefineMethod (method.MemberName,
                                                                     methodAttr,
                                                                     GetType (method.ReturnType),
                                                                     pTypes);

                    // then define parameter infos
                    for (j = 0; j < method.Parameters; j ++) {
                        ParameterAttributes attr = 0;
                        if (parameters [j].IsIn)
                            attr |= ParameterAttributes.In;
                        if (parameters [j].IsOut)
                            attr |= ParameterAttributes.Out;

                        if (attr == 0)
                            throw new Exception ("wrong parameter attributes");

                        methodBuilder.DefineParameter (parameters [j].Position + 1,
                                                       attr,
                                                       parameters [j].Name);
                    }

                    if (method.ReturnsStruct) {
                        // Console.WriteLine ("returns struct");

                        UnoXStructTypeDescription std = method.ReturnType as UnoXStructTypeDescription;
                        if (std.TypeArguments != 0) {
                            // Console.WriteLine ("std: '{0}' '{1}' {2}", std, method.ReturnType, method.ReturnType.Name);
                            Type[] ats = new Type [std.TypeArguments];

                            // Console.WriteLine ("type arguments: {0}", std.TypeArguments);

                            for (j = 0; j < std.TypeArguments; j ++) {
                                ats [j] = GetType (std.TypeArgument (j));
                                //Console.WriteLine ("ats [{0}] = {1}", j, ats [j]);
                            }

                            object[] atso = { ats };
                            Type[] ctor = { typeof (Type[]) };
                            attrBuilder = new CustomAttributeBuilder (typeof (uno.TypeArgumentsAttribute).GetConstructor (ctor),
                                                                  atso);
                            methodBuilder.SetCustomAttribute (attrBuilder);
                        }
                    }

                    // define UNO exception attribute (exceptions) --------------------------------------
                    attrBuilder = IFaceMethodExceptionAttribute (method);
                    if (attrBuilder != null)
                        methodBuilder.SetCustomAttribute (attrBuilder);

                    // oneway attribute
                    if (method.IsOneway) {
                        Type[] arCtorOneway = new Type [0];
                        object[] arArgs = new object [0];
                        attrBuilder = new CustomAttributeBuilder (typeof (uno.OnewayAttribute).GetConstructor (arCtorOneway),
                                                                  arArgs);
                        methodBuilder.SetCustomAttribute (attrBuilder);
                    }
                } else {
                    // attribute
                    if (member.TypeClass != UnoTypeClass.InterfaceAttribute)
                        throw new Exception (String.Format ("Unknown member type class: {0} ", member.TypeClass));

                    UnoXInterfaceAttributeTypeDescription attribute = new UnoXInterfaceAttributeTypeDescription (member.Handle);
                    const MethodAttributes propMethodAttr = methodAttr | MethodAttributes.SpecialName;

                    Type attrType = GetType (attribute.Type);
                    //Console.WriteLine ("attribute {2} type: {0} => {1}", attribute.Type, attrType, attribute.Name);
                    Type[] parameters = new Type [0];

                    PropertyBuilder propBuilder = entry.mTypeBuilder.DefineProperty (attribute.MemberName, PropertyAttributes.None, attrType, parameters);

                    //set BoundAttribute, if necessary
                    if (attribute.IsBound) {
                        ConstructorInfo ctorBoundAttr = typeof (uno.BoundAttribute).GetConstructor (new Type [0]);
                        attrBuilder = new CustomAttributeBuilder (ctorBoundAttr, new object [0]);
                        propBuilder.SetCustomAttribute (attrBuilder);
                    }

                    // getter
                    methodBuilder = entry.mTypeBuilder.DefineMethod ("get_" + attribute.MemberName,
                                                                     propMethodAttr, attrType, parameters);

                    attrBuilder = ExceptionAttribute (GetTypes (attribute.GetExceptionTypes));
                    if (attrBuilder != null)
                        methodBuilder.SetCustomAttribute (attrBuilder);

                    propBuilder.SetGetMethod (methodBuilder);

                    if (!attribute.IsReadOnly) {
                        // setter
                        parameters = new Type [1];
                        parameters [0] = attrType;
                        //parameters [0] = null;
                        //Console.WriteLine ("setter parameters: {0} ({1})", parameters, parameters [0]);
                        methodBuilder = entry.mTypeBuilder.DefineMethod ("set_" + attribute.MemberName,
                                                                         propMethodAttr, typeof (void), parameters);
                        methodBuilder.DefineParameter (1, ParameterAttributes.In, "value");

                        attrBuilder = ExceptionAttribute (GetTypes (attribute.SetExceptionTypes));
                        if (attrBuilder != null)
                            methodBuilder.SetCustomAttribute (attrBuilder);

                        propBuilder.SetSetMethod (methodBuilder);
                    }
                }
            }
            //	cmm_x_interface_type_description_members_release (membersHandle);

            if (mConfig.mVerbose)
                Console.WriteLine ("> emitting interface type {0}", "unoidl." + entry.mType.Name);

            mIncompleteIFaces.Remove (entry.mTypeBuilder.FullName);

            //Console.WriteLine ("completed {0}", entry.mTypeBuilder.FullName);

            return entry.mTypeBuilder.CreateType ();
    }

    UnoXInterfaceTypeDescription ResolveInterfaceTypedef (UnoXTypeDescription xtd)
    {
        UnoXInterfaceTypeDescription xtdIface = xtd as UnoXInterfaceTypeDescription;

        if (xtdIface != null)
            return xtdIface;

        UnoXIndirectTypeDescription xtdIndirect = xtd as UnoXIndirectTypeDescription;

        if (xtdIndirect != null)
            return ResolveInterfaceTypedef (xtdIndirect.ReferencedType);;

        throw new Exception ("resolveInterfaceTypedef was called with an invalid argument");
    }

    ArrayList GetServiceConstructorMethodExceptionsReduced (UnoXTypeDescription[] exceptionTypes)
    {
        if (exceptionTypes.Length == 0)
            return new ArrayList ();

        ArrayList types = new ArrayList();
        int i;

        for (i = 0; i < exceptionTypes.Length; i ++)
            types.Add (GetType ("unoidl." + exceptionTypes [i].Name, true));

        int start = 0;
        while (true) {
            bool bRemove = false;

            for (i = start; i < types.Count; i ++) {
                Type t = types [i] as Type;
                int j;

                for (j = 0; j < types.Count; j ++) {
                    if (t.IsSubclassOf (types [j] as Type)) {
                        types.RemoveAt (i);
                        bRemove = true;
                        break;
                    }
                }
                if (bRemove)
                    break;
                start ++;
            }

            if (bRemove == false)
                break;
        }

        return types;
    }

    Type CompleteServiceType (ServiceEntry entry)
    {
        // Create the private default constructor
        ConstructorBuilder ctorBuilder = entry.mTypeBuilder.DefineConstructor (MethodAttributes.Private |
                                                                               MethodAttributes.HideBySig |
                                                                               MethodAttributes.SpecialName |
                                                                               MethodAttributes.RTSpecialName,
                                                                               CallingConventions.Standard,
                                                                               null);

        ILGenerator ilGen = ctorBuilder.GetILGenerator ();
        ilGen.Emit (OpCodes.Ldarg_0);  // push this
        ilGen.Emit (OpCodes.Call, entry.mTypeBuilder.BaseType.GetConstructor (new Type[0]));
        ilGen.Emit (OpCodes.Ret);

        // Create the service constructors.
        // obtain the interface which makes up this service, it is the return
        // type of the constructor functions
        UnoXInterfaceTypeDescription iface = entry.mType.Interface as UnoXInterfaceTypeDescription;

        if (iface == null)
            iface = ResolveInterfaceTypedef (entry.mType.Interface);

        Type ret = GetType (iface);

        // Create the ConstructorInfo for a DeploymentException
        Type typeDeploymentException = GetType ("unoidl.com.sun.star.uno.DeploymentException", true);
        Type[] arTypeCtor = { typeof (string), typeof (object) };
        ConstructorInfo ctorDeploymentException = typeDeploymentException.GetConstructor (arTypeCtor);

        Type typeUnoException = GetType ("unoidl.com.sun.star.uno.Exception", true);
        int i, j;

        for (i = entry.mType.Constructors - 1; i >= 0; i --) {
            bool bParameterArray = false;
            UnoXServiceConstructorDescription ctorDesc = entry.mType.Constructor (i);
            Type[] typeParameters = new Type [ctorDesc.Parameters + 1];
            typeParameters [0] = GetType ("unoidl.com.sun.star.uno.XComponentContext", true);

            for (j = 0; j < ctorDesc.Parameters; j ++) {
                UnoXParameter parameter = ctorDesc.Parameter (j);

                if (parameter.IsRest)
                    typeParameters [j + 1] = typeof (uno.Any[]);
                else
                    typeParameters [j + 1] = GetType (parameter.Type);
            }

            // The array typeParameters can contain:
            // System.Type and uno.PolymorphicType.
            // Passing PolymorphicType to MethodBuilder.DefineMethod will cause a problem.
            // The exception will read something like no on information for parameter # d
            // Maybe we need no override another Type method in PolymorphicType ...
            // Until we have figured this out, we will create another array of System.Type which
            // we pass on to DefineMethod.

            Type[] paramTypes = new Type [ctorDesc.Parameters + 1];

            for (j = 0; j < ctorDesc.Parameters + 1; j ++) {
                if (typeParameters [j] is uno.PolymorphicType)
                    paramTypes [j] = (typeParameters [j] as uno.PolymorphicType).OriginalType;
                else
                    paramTypes [j] = typeParameters [j];
            }

            //define method
            string ctorName;
            if (ctorDesc.IsDefault)
                ctorName = "create";
            else
                ctorName = ctorDesc.Name;

            MethodBuilder methodBuilder = entry.mTypeBuilder.DefineMethod (ctorName,
                                                                           MethodAttributes.Public |
                                                                           MethodAttributes.HideBySig |
                                                                           MethodAttributes.Static,
                                                                           ret,
                                                                           paramTypes);

            //define UNO exception attribute (exceptions)--------------------------------------
            CustomAttributeBuilder attrBuilder = ExceptionAttribute (GetTypes (ctorDesc.ExceptionTypes));
            if (attrBuilder != null)
                methodBuilder.SetCustomAttribute (attrBuilder);

            // define parameter attributes (paramarray), names etc.
            // The first parameter is the XComponentContext, which cannot be obtained
            // from reflection.
            // The context is not part of the idl description

            methodBuilder.DefineParameter (1, ParameterAttributes.In, "the_context");

            ParameterBuilder[] parameterBuilder = new ParameterBuilder [ctorDesc.Parameters];
            int iparam;

            for (iparam = 0; iparam < ctorDesc.Parameters; iparam ++) {
                UnoXParameter parameter = ctorDesc.Parameter (iparam);

                parameterBuilder [iparam] = methodBuilder.DefineParameter (iparam + 2, ParameterAttributes.In, parameter.Name);

                if (parameter.IsRest) {
                    bParameterArray = true;
                    //set the ParameterArrayAttribute
                    ConstructorInfo ctorInfo = typeof (System.ParamArrayAttribute).GetConstructor (new Type [0]);
                    attrBuilder = new CustomAttributeBuilder (ctorInfo, new object [0]);

                    parameterBuilder[iparam].SetCustomAttribute (attrBuilder);
                    break;
                }
            }

            ilGen = methodBuilder.GetILGenerator ();

            // Define locals ---------------------------------
            // XMultiComponentFactory
            LocalBuilder localFactory = ilGen.DeclareLocal (GetType ("unoidl.com.sun.star.lang.XMultiComponentFactory", true));

            // The return type
            LocalBuilder localReturnVal = ilGen.DeclareLocal (ret);

            // Obtain the XMultiComponentFactory and throw an exception if we do not get one
            ilGen.Emit (OpCodes.Ldarg_0);

            MethodInfo methodGetServiceManager = GetType ("unoidl.com.sun.star.uno.XComponentContext", true).GetMethod ("getServiceManager");
            ilGen.Emit (OpCodes.Callvirt, methodGetServiceManager);
            ilGen.Emit (OpCodes.Stloc, localFactory);
            ilGen.Emit (OpCodes.Ldloc, localFactory);
            Label label1 = ilGen.DefineLabel ();
            ilGen.Emit (OpCodes.Brtrue, label1);

            // The string for the exception
            System.Text.StringBuilder strBuilder = new System.Text.StringBuilder (256);

            strBuilder.Append ("The service ");
            strBuilder.Append ("unoidl." + entry.mType.Name);
            strBuilder.Append (" could not be created. The context failed to supply the service manager.");

            ilGen.Emit (OpCodes.Ldstr, strBuilder.ToString ());
            ilGen.Emit (OpCodes.Ldarg_0);
            ilGen.Emit (OpCodes.Newobj, ctorDeploymentException);
            ilGen.Emit (OpCodes.Throw);
            ilGen.MarkLabel (label1);

            // We create a try/ catch around the createInstanceWithContext, etc. functions
            // There are 3 cases
            // 1. function do not specify exceptions. Then RuntimeExceptions are retrhown and other
            //    exceptions produce a DeploymentException.
            // 2. function specify  Exception. Then all exceptions fly through
            // 3. function specifies exceptions but no Exception. Then these are rethrown
            //    and other exceptions, except RuntimeException, produce a deployment exception.
            // In case there are no parameters we call
            // XMultiComponentFactory.createInstanceWithContext

            ArrayList exceptionTypes = GetServiceConstructorMethodExceptionsReduced (ctorDesc.ExceptionTypes);
            if (!exceptionTypes.Contains (typeUnoException)) {
                ilGen.BeginExceptionBlock ();
            }

            if (ctorDesc.Parameters == 0) {
                ilGen.Emit (OpCodes.Ldloc, localFactory);
                ilGen.Emit (OpCodes.Ldstr, entry.mType.Name);
                ilGen.Emit (OpCodes.Ldarg_0);

                MethodInfo methodCreate = localFactory.LocalType.GetMethod ("createInstanceWithContext");
                ilGen.Emit (OpCodes.Callvirt, methodCreate);
            } else if(bParameterArray) {
                //Service constructor with parameter array
                ilGen.Emit (OpCodes.Ldloc, localFactory);
                ilGen.Emit (OpCodes.Ldstr, entry.mType.Name);
                ilGen.Emit (OpCodes.Ldarg_1);
                ilGen.Emit (OpCodes.Ldarg_0);
                MethodInfo methodCreate = localFactory.LocalType.GetMethod ("createInstanceWithArgumentsAndContext");
                ilGen.Emit(OpCodes.Callvirt, methodCreate);
            } else {
                // Any param1, Any param2, etc.
                // For each parameter,except the component context, and parameter array
                // and Any is created.
                LocalBuilder[] localAny = new LocalBuilder [ctorDesc.Parameters];

                for (iparam = 0; iparam < ctorDesc.Parameters; iparam ++) {
                    localAny [iparam] = ilGen.DeclareLocal (typeof (uno.Any));
                }

                // Any[]. This array is filled with the created Anys which contain the parameters
                // and the values contained in the parameter array
                LocalBuilder localAnyParams = ilGen.DeclareLocal (typeof (uno.Any []));

                // Create the Any for every argument, except for the parameter array
                // arLocalAny contains the LocalBuilder for all these parameters.
                // we call the ctor Any(Type, Object)
                // If the parameter is an Any then the Any is created with Any(param.Type, param.Value);
                Type[] typesCtorAny = { typeof (Type),
                                        typeof (object) };
                ConstructorInfo ctorAny = typeof (uno.Any).GetConstructor (typesCtorAny);
                MethodInfo methodAnyGetType = typeof (uno.Any).GetProperty ("Type").GetGetMethod ();
                MethodInfo methodAnyGetValue = typeof (uno.Any).GetProperty ("Value").GetGetMethod ();

                for (j = 0; j < localAny.Length; j ++) {
                    //check if the parameter is a polymorphic struct
                    if (typeParameters [j + 1] is uno.PolymorphicType) {
                        // It is a polymorphic struct
                        uno.PolymorphicType polyType = typeParameters [j + 1] as uno.PolymorphicType;

                        // Load the uninitialized local Any on which we will call the ctor
                        ilGen.Emit (OpCodes.Ldloca, localAny [j]);

                        // Call PolymorphicType PolymorphicType::GetType(Type t, String polyName)
                        // Prepare the first parameter
                        ilGen.Emit (OpCodes.Ldtoken, polyType.OriginalType);
                        Type[] typeParams = { typeof (System.RuntimeTypeHandle) };
                        ilGen.Emit (OpCodes.Call, typeof(Type).GetMethod ("GetTypeFromHandle", typeParams));

                        // Prepare the second parameter
                        ilGen.Emit (OpCodes.Ldstr, polyType.PolymorphicName);

                        // Make the actual call
                        Type[] typeParamGetType = { typeof (Type), typeof (string) };
                        ilGen.Emit (OpCodes.Call,
                                    typeof (uno.PolymorphicType).GetMethod("GetType",
                                                                           typeParamGetType));

                        // Stack is: localAny, PolymorphicType
                        // Call Any::Any(Type, Object)
                        // Prepare the second parameter for the any ctor
                        ilGen.Emit (OpCodes.Ldarg, j + 1);

                        // if the parameter is a value type then we need to box it, because
                        // the Any ctor takes an Object
                        if (typeParameters [j + 1].IsValueType)
                            ilGen.Emit (OpCodes.Box, typeParameters [j + 1]);
                        ilGen.Emit (OpCodes.Call, ctorAny);
                    } else if (typeParameters [j + 1] == typeof (uno.Any)) {
                        // Create the call new Any(param.Type,param,Value)
                        // Stack must be Any,Type,Value
                        // First load the Any which is to be constructed
                        ilGen.Emit (OpCodes.Ldloca, localAny [j]);

                        //Load the Type, which is obtained by calling param.Type
                        ilGen.Emit (OpCodes.Ldarga, j + 1);
                        ilGen.Emit (OpCodes.Call, methodAnyGetType);

                        //Load the Value, which is obtained by calling param.Value
                        ilGen.Emit (OpCodes.Ldarga, j + 1);
                        ilGen.Emit (OpCodes.Call, methodAnyGetValue);

                        //Call the Any ctor.
                        ilGen.Emit (OpCodes.Call, ctorAny);
                    } else {
                        ilGen.Emit (OpCodes.Ldloca, localAny [j]);
                        ilGen.Emit (OpCodes.Ldtoken, typeParameters [j + 1]);

                        Type[] typeParams = { typeof (System.RuntimeTypeHandle) };
                        ilGen.Emit (OpCodes.Call, typeof (Type).GetMethod ("GetTypeFromHandle", typeParams));
                        ilGen.Emit(OpCodes.Ldarg, j + 1);

                        // if the parameter is a value type then we need to box it, because
                        // the Any ctor takes an Object
                        if (typeParameters [j + 1].IsValueType)
                            ilGen.Emit (OpCodes.Box, typeParameters [j + 1]);
                        ilGen.Emit(OpCodes.Call, ctorAny);
                    }
                }


                // Create the Any[] that is passed to the
                // createInstanceWithContext[AndArguments] function
                ilGen.Emit (OpCodes.Ldc_I4, localAny.Length);
                ilGen.Emit (OpCodes.Newarr, typeof (uno.Any));
                ilGen.Emit (OpCodes.Stloc, localAnyParams);

                // Assign all anys created from the parameters
                // array to the Any[]
                for (j = 0; j < localAny.Length; j ++) {
                    ilGen.Emit (OpCodes.Ldloc, localAnyParams);
                    ilGen.Emit (OpCodes.Ldc_I4, j);
                    ilGen.Emit (OpCodes.Ldelema, typeof (uno.Any));
                    ilGen.Emit (OpCodes.Ldloc, localAny [j]);
                    ilGen.Emit (OpCodes.Stobj, typeof (uno.Any));
                }

                // call createInstanceWithContextAndArguments
                ilGen.Emit (OpCodes.Ldloc, localFactory);
                ilGen.Emit (OpCodes.Ldstr, entry.mType.Name);
                ilGen.Emit (OpCodes.Ldloc, localAnyParams);
                ilGen.Emit (OpCodes.Ldarg_0);
                MethodInfo methodCreate = localFactory.LocalType.GetMethod ("createInstanceWithArgumentsAndContext");
                ilGen.Emit (OpCodes.Callvirt, methodCreate);
            }

            // cast the object returned by the functions createInstanceWithContext or
            // createInstanceWithArgumentsAndContext to the interface type
            ilGen.Emit (OpCodes.Castclass, ret);
            ilGen.Emit (OpCodes.Stloc, localReturnVal);

            //catch exceptions thrown by createInstanceWithArgumentsAndContext and createInstanceWithContext
            if (!exceptionTypes.Contains (typeUnoException)) {
                // catch (unoidl.com.sun.star.uno.RuntimeException) {throw;}
                ilGen.BeginCatchBlock (GetType ("unoidl.com.sun.star.uno.RuntimeException", true));
                ilGen.Emit (OpCodes.Pop);
                ilGen.Emit (OpCodes.Rethrow);

                //catch and rethrow all other defined Exceptions
                for (j = 0; j < exceptionTypes.Count; j ++) {
                    Type excType = exceptionTypes [j] as Type;
                    if (excType.IsInstanceOfType (GetType ("unoidl.com.sun.star.uno.RuntimeException", true))) {
                        // we have a catch for RuntimeException already defined
                        continue;
                    }

                    //catch Exception and rethrow
                    ilGen.BeginCatchBlock (excType);
                    ilGen.Emit (OpCodes.Pop);
                    ilGen.Emit (OpCodes.Rethrow);
                }

                //catch (unoidl.com.sun.star.uno.Exception) {throw DeploymentException...}
                ilGen.BeginCatchBlock (typeUnoException);

                //Define the local variabe that keeps the exception
                LocalBuilder localException = ilGen.DeclareLocal (typeUnoException);

                //Store the exception
                ilGen.Emit (OpCodes.Stloc, localException);

                //prepare the construction of the exception
                strBuilder = new System.Text.StringBuilder (256);
                strBuilder.Append ("The context (com.sun.star.uno.XComponentContext) failed to supply the service ");
                strBuilder.Append ("unoidl." + entry.mType.Name);
                strBuilder.Append (": ");

                ilGen.Emit (OpCodes.Ldstr, strBuilder.ToString());

                // add to the string the Exception.Message
                ilGen.Emit (OpCodes.Ldloc, localException);
                //Console.WriteLine ("get message property of type: {0}", typeUnoException);
                ilGen.Emit (OpCodes.Callvirt, typeUnoException.GetProperty ("Message").GetGetMethod ());
                Type[] concatParams = { typeof (string), typeof (string)};
                ilGen.Emit (OpCodes.Call, typeof (string).GetMethod ("Concat", concatParams));

                //load contex argument
                ilGen.Emit (OpCodes.Ldarg_0);
                ilGen.Emit (OpCodes.Newobj, ctorDeploymentException);
                ilGen.Emit (OpCodes.Throw); //Exception(typeDeploymentExc);

                ilGen.EndExceptionBlock();
            }


            // Check if the service instance was create and throw a exception if not.
            Label labelServiceCreated = ilGen.DefineLabel ();
            ilGen.Emit (OpCodes.Ldloc, localReturnVal);
            ilGen.Emit (OpCodes.Brtrue_S, labelServiceCreated);

            strBuilder = new System.Text.StringBuilder(256);
            strBuilder.Append ("The context (com.sun.star.uno.XComponentContext) failed to supply the service ");
            strBuilder.Append ("unoidl." + entry.mType.Name);
            strBuilder.Append (".");
            ilGen.Emit (OpCodes.Ldstr, strBuilder.ToString());
            ilGen.Emit (OpCodes.Ldarg_0);
            ilGen.Emit (OpCodes.Newobj, ctorDeploymentException);
            ilGen.Emit (OpCodes.Throw); //Exception(typeDeploymentExc);

            ilGen.MarkLabel (labelServiceCreated);
            ilGen.Emit (OpCodes.Ldloc, localReturnVal);
            ilGen.Emit (OpCodes.Ret);
        }

        mIncompleteServices.Remove (entry.mTypeBuilder.FullName);

		if (mConfig.mVerbose)
			Console.WriteLine("> emitting service type {0}", "unoidl." + entry.mType.Name);

        //Console.WriteLine ("completed service {0}", entry.mTypeBuilder.FullName);

        return entry.mTypeBuilder.CreateType ();
    }

    Type CompleteSingletonType (SingletonEntry entry)
    {
        string name = "unoidl." + entry.mType.Name;

        // Create the private default constructor
        ConstructorBuilder ctorBuilder = entry.mTypeBuilder.DefineConstructor (MethodAttributes.Private |
                                                                               MethodAttributes.HideBySig |
                                                                               MethodAttributes.SpecialName |
                                                                               MethodAttributes.RTSpecialName,
                                                                               CallingConventions.Standard, null);

        ILGenerator ilGen = ctorBuilder.GetILGenerator ();
        ilGen.Emit (OpCodes.Ldarg_0); // push this
        ilGen.Emit (OpCodes.Call, entry.mTypeBuilder.BaseType.GetConstructor (new Type [0]));
        ilGen.Emit (OpCodes.Ret);

        // obtain the interface which makes up this service, it is the return
        // type of the constructor functions
        UnoXTypeDescription ifaceTD = entry.mType.Interface;
        if (!(ifaceTD is UnoXInterfaceTypeDescription))
            ifaceTD = ResolveInterfaceTypedef (ifaceTD);
        Type retType = GetType (ifaceTD);

        //define method
        Type[]  typeParameters = { GetType ("unoidl.com.sun.star.uno.XComponentContext", true) };
        MethodBuilder methodBuilder = entry.mTypeBuilder.DefineMethod ("get", MethodAttributes.Public |
                                                                       MethodAttributes.HideBySig |
                                                                       MethodAttributes.Static,
                                                                       retType,
                                                                       typeParameters);

        // The first parameter is the XComponentContext, which cannot be obtained
        // from reflection.
        // The context is not part of the idl description
        methodBuilder.DefineParameter (1, ParameterAttributes.In, "the_context");

        ilGen = methodBuilder.GetILGenerator();
        // Define locals ---------------------------------
        // Any, returned by XComponentContext.getValueByName
        LocalBuilder localAny = ilGen.DeclareLocal (typeof (uno.Any));

        // Call XContext.getValueByName
        ilGen.Emit (OpCodes.Ldarg_0);

        // build the singleton name : /singleton/unoidl.com.sun.star.XXX
        ilGen.Emit(OpCodes.Ldstr, "/singletons/" + name);

        MethodInfo methodGetValueByName = GetType ("unoidl.com.sun.star.uno.XComponentContext", true).GetMethod ("getValueByName");
        ilGen.Emit(OpCodes.Callvirt, methodGetValueByName);
        ilGen.Emit(OpCodes.Stloc_0);

        //Contains the returned Any a value?
        ilGen.Emit(OpCodes.Ldloca_S, localAny);
        MethodInfo methodHasValue = typeof (uno.Any).GetMethod ("hasValue");
        ilGen.Emit (OpCodes.Call, methodHasValue);

        //If not, then throw an DeploymentException
        Label labelSingletonExists = ilGen.DefineLabel ();
        ilGen.Emit (OpCodes.Brtrue_S, labelSingletonExists);
        ilGen.Emit (OpCodes.Ldstr, "Component context fails to supply singleton " + name + " of type " + retType.FullName + ".");
        ilGen.Emit (OpCodes.Ldarg_0);
        Type[] typesCtorDeploymentException = { typeof (string), typeof (object) };
        ilGen.Emit (OpCodes.Newobj, GetType ("unoidl.com.sun.star.uno.DeploymentException", true).GetConstructor (typesCtorDeploymentException));
        ilGen.Emit (OpCodes.Throw);
        ilGen.MarkLabel (labelSingletonExists);

        //Cast the singleton contained in the Any to the expected interface and return it.
        ilGen.Emit (OpCodes.Ldloca_S, localAny);
        ilGen.Emit (OpCodes.Call, typeof (uno.Any).GetProperty ("Value").GetGetMethod ());
        ilGen.Emit (OpCodes.Castclass, retType);
        ilGen.Emit (OpCodes.Ret);

        if (mConfig.mVerbose)
            Console.WriteLine ("> emitting singleton type {0}", name);

        mIncompleteSingletons.Remove (entry.mTypeBuilder.FullName);

        //Console.WriteLine ("completed singleton {0}", entry.mTypeBuilder.FullName);

        return entry.mTypeBuilder.CreateType ();
    }

    public void Dispose ()
    {
        while (mIncompleteIFaces.Count > 0) {
            IDictionaryEnumerator e = mIncompleteIFaces.GetEnumerator ();
            e.MoveNext ();
            CompleteIFaceType (e.Value as IFaceEntry);
        }

        while (mIncompleteServices.Count > 0) {
            IDictionaryEnumerator e = mIncompleteServices.GetEnumerator ();
            e.MoveNext ();
            CompleteServiceType (e.Value as ServiceEntry);
        }

        while (mIncompleteSingletons.Count > 0) {
            IDictionaryEnumerator e = mIncompleteSingletons.GetEnumerator ();
            e.MoveNext ();
            CompleteSingletonType (e.Value as SingletonEntry);
        }
    }
}

class Config
{
    public ArrayList mMandatoryRegistries = new ArrayList ();
    public ArrayList mExplicitTypes = new ArrayList ();
    public ArrayList mExtraAssemblies = new ArrayList ();
    public ArrayList mExtraRegistries = new ArrayList ();
    public bool mVerbose = false;
    public string
        mOutput = null,
        mVersion = null,
        mDescription = null,
        mProduct = null,
        mCompany = null,
        mCopyright = null,
        mTrademark = null,
        mKeyfile = null,
        mDelaySign = null;
}

class CliMaker
{
    IntPtr mUnoHelper;

//     string mOutputDir = "./";
//     string mOutputFile = "cli_uretypes.dll";
//     string mName = "cli_uretypes";

    string mOutputDir;
    string mOutputFile;
    string mName;

    Config mConfig;

    AssemblyBuilder mAssemblyBuilder;
    ResolveEventHandler mTypeResolver = null;

    public CliMaker (Config config)
    {
        mConfig = config;
        mUnoHelper = cmm_uno_helper_new ();
    }

    ~CliMaker ()
    {
        cmm_uno_helper_delete (mUnoHelper);
    }

    public void OpenRegistries ()
    {
        if (mConfig.mExplicitTypes.Count > 0)
            foreach (string typeName in mConfig.mExplicitTypes)
                cmm_uno_helper_add_explicit_type (mUnoHelper, typeName);

        foreach (string registry in mConfig.mMandatoryRegistries)
            cmm_uno_helper_add_mandatory_registry (mUnoHelper, registry);

        foreach (string registry in mConfig.mExtraRegistries)
            cmm_uno_helper_add_extra_registry (mUnoHelper, registry);

        cmm_uno_helper_open_registries (mUnoHelper);
    }

    public void PrepareAssembly ()
    {
        // Get the key pair for making a strong name
        StrongNameKeyPair kp = null;
        if (mConfig.mKeyfile != null) {
            try {
                System.IO.FileStream fs = new System.IO.FileStream (mConfig.mKeyfile, System.IO.FileMode.Open);
                kp = new StrongNameKeyPair (fs);
                fs.Close ();
            } catch (System.IO.FileNotFoundException) {
                throw new Exception ("Could not find the keyfile. Verify the --keyfile argument!");
            }
        } else {
            if (mConfig.mVerbose)
                Console.WriteLine ("> no key file specified. Cannot create strong name!");
        }

        mOutputFile = System.IO.Path.GetFileName (mConfig.mOutput);
        mOutputDir = System.IO.Path.GetDirectoryName (mConfig.mOutput);
        mName = System.IO.Path.GetFileNameWithoutExtension (mConfig.mOutput);
// 		int idx = mOutputFile.LastIndexOf (System.IO.Path.DirectorySeparatorChar);
// 		if (idx >= 0)
// 			mOutputDir = mOutputFile.Substring (0, idx);
// 		else
// 			mOutputDir = System.IO.Path.CurrentDirectory;
// 		idx = mOutputFile.LastIndexOf (".dll");
// 		mName = mOutputFile.Substring (0, idx);
        //Console.WriteLine ("file {0}\ndir  {1}\nname {2}", mOutputFile, mOutputDir, mName);

        // setup assembly info: xxx todo set more? e.g. avoid strong versioning
        AssemblyName assemblyName = new AssemblyName();
        assemblyName.CodeBase = mOutputDir;
        assemblyName.Name = mName;

        if (kp != null)
            assemblyName.KeyPair = kp;

        if (mConfig.mVersion != null)
            assemblyName.Version = new System.Version (mConfig.mVersion);

        // target assembly
        mAssemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly (assemblyName, AssemblyBuilderAccess.Save, mOutputDir);

        if (mConfig.mProduct != null) {
            Type[] paramTypes = { typeof (string) };
            object[] args = { mConfig.mProduct };
            mAssemblyBuilder.SetCustomAttribute (new CustomAttributeBuilder (typeof (AssemblyProductAttribute).GetConstructor (paramTypes), args));
        }

        if (mConfig.mDescription != null) {
            Type[] paramTypes = { typeof (string) };
            object[] args = { mConfig.mDescription };
            mAssemblyBuilder.SetCustomAttribute (new CustomAttributeBuilder (typeof (AssemblyDescriptionAttribute).GetConstructor (paramTypes), args));
        }

        if (mConfig.mCompany != null) {
            Type[] paramTypes = { typeof (string) };
            object[] args = { mConfig.mCompany };
            mAssemblyBuilder.SetCustomAttribute (new CustomAttributeBuilder (typeof (AssemblyCompanyAttribute).GetConstructor (paramTypes), args));
        }

        if (mConfig.mCopyright != null) {
            Type[] paramTypes = { typeof (string) };
            object[] args = { mConfig.mCopyright };
            mAssemblyBuilder.SetCustomAttribute (new CustomAttributeBuilder (typeof (AssemblyCopyrightAttribute).GetConstructor (paramTypes), args));
        }

        if (mConfig.mTrademark != null) {
            Type[] paramTypes = { typeof (string) };
            object[] args = { mConfig.mTrademark };
            mAssemblyBuilder.SetCustomAttribute (new CustomAttributeBuilder (typeof (AssemblyTrademarkAttribute).GetConstructor (paramTypes), args));
        }
    }

    public UnoXTypeDescription NextType ()
    {
        IntPtr handle = cmm_uno_helper_next_type (mUnoHelper);

        if (handle != IntPtr.Zero) {
            return UnoXTypeDescription.Create (handle);
        }

        return null;
    }

    public void Emit ()
    {
        TypeEmitter mTypeEmitter = new TypeEmitter (mConfig, mAssemblyBuilder.DefineDynamicModule( mOutputFile ));

        AppDomain.CurrentDomain.TypeResolve += mTypeEmitter.ResolveEventHandler;

        UnoXTypeDescription type;

        while ((type = NextType ()) != null)
                mTypeEmitter.GetType (type);

        mTypeEmitter.Dispose ();
    }

    public void Save ()
    {
         if (mConfig.mVerbose)
             Console.Write ("> saving assembly {0}{1}{2}...",
                            mOutputDir,
                            System.IO.Path.DirectorySeparatorChar,
                            mOutputFile);

         mAssemblyBuilder.Save (mOutputFile);

         if (mConfig.mVerbose)
             Console.WriteLine ("ok.");

         AppDomain.CurrentDomain.TypeResolve -= mTypeResolver;
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_uno_helper_new ();

    [DllImport("climaker")]
    private static extern void cmm_uno_helper_delete (IntPtr handle);

    [DllImport("climaker")]
    private static extern void cmm_uno_helper_add_mandatory_registry (IntPtr handle, string registry);

    [DllImport("climaker")]
    private static extern void cmm_uno_helper_add_extra_registry (IntPtr handle, string registry);

    [DllImport("climaker")]
    private static extern void cmm_uno_helper_add_explicit_type (IntPtr handle, string typeName);

    [DllImport("climaker")]
    private static extern void cmm_uno_helper_open_registries (IntPtr handle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_uno_helper_next_type (IntPtr handle);
}

class UnoObject
{
    protected IntPtr mHandle;

    protected UnoObject (IntPtr handle)
    {
        mHandle = handle;
    }

    public IntPtr Handle
    {
        get
        {
            return mHandle;
        }
    }
}

class UnoXTypeDescription : UnoObject
{
    protected UnoXTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public static UnoXTypeDescription Create (IntPtr handle)
    {
        //Console.WriteLine ("Create {0}", handle);
        if (handle == IntPtr.Zero)
            return null;

        UnoTypeClass tc = cmm_x_type_description_get_type_class (handle);
        switch (tc) {
        case UnoTypeClass.Enum:
            return new UnoXEnumTypeDescription (handle);
        case UnoTypeClass.Interface:
            return new UnoXInterfaceTypeDescription (handle);
        case UnoTypeClass.Struct:
            return new UnoXStructTypeDescription (handle);
        case UnoTypeClass.Exception:
            return new UnoXExceptionTypeDescription (handle);
        case UnoTypeClass.Sequence:
        case UnoTypeClass.Typedef:
            return new UnoXIndirectTypeDescription (handle);
        case UnoTypeClass.Constant:
            return new UnoXConstantTypeDescription (handle);
        case UnoTypeClass.Constants:
            return new UnoXConstantsTypeDescription (handle);
        case UnoTypeClass.Service:
            return new UnoXServiceTypeDescription (handle);
        case UnoTypeClass.Singleton:
            return new UnoXSingletonTypeDescription (handle);
        }

        //Console.WriteLine ("unknown type {0}", tc);

        return new UnoXTypeDescription (handle);
    }

    public string Name
    {
        get
        {
            return Marshal.PtrToStringAnsi (cmm_x_type_description_get_name (mHandle));
        }
    }

    public UnoTypeClass TypeClass
    {
        get
        {
            return cmm_x_type_description_get_type_class (mHandle);
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_type_description_get_name (IntPtr handle);

    [DllImport("climaker")]
    private static extern UnoTypeClass cmm_x_type_description_get_type_class (IntPtr handle);
}

class UnoXEnumTypeDescription : UnoXTypeDescription
{
    public UnoXEnumTypeDescription (IntPtr mHandle) : base (mHandle)
    {
    }

    public int Length
    {
        get
        {
            return cmm_x_enum_type_description_get_length (mHandle);
        }
    }

    public string ValueName (int idx)
    {
        return Marshal.PtrToStringAnsi (cmm_x_enum_type_description_get_name (mHandle, idx));
    }

    public int Value (int idx)
    {
        return cmm_x_enum_type_description_get_value (mHandle, idx);
    }

    [DllImport("climaker")]
    private static extern int cmm_x_enum_type_description_get_length (IntPtr handle);

    [DllImport("climaker")]
    private static extern int cmm_x_enum_type_description_get_value (IntPtr handle, int idx);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_enum_type_description_get_name (IntPtr handle, int idx);
}

class UnoXInterfaceTypeDescription : UnoXTypeDescription
{
    int mMembers = 0;
    IntPtr mMembersHandle = IntPtr.Zero;

    public UnoXInterfaceTypeDescription (IntPtr mHandle) : base (mHandle)
    {
    }

    public int BaseTypes
    {
        get
        {
            return cmm_x_interface_type_description_get_length (mHandle);
        }
    }

    public UnoXInterfaceTypeDescription BaseType (int idx)
    {
        return new UnoXInterfaceTypeDescription (cmm_x_interface_type_description_get_base_type (mHandle, idx));
    }

    void AssureMembers ()
    {
        if (mMembersHandle == IntPtr.Zero)
            mMembers = cmm_x_interface_type_description_get_members (mHandle, out mMembersHandle);
    }

    public int Members
    {
        get
        {
            AssureMembers ();

            return mMembers;
        }
    }

    public UnoXInterfaceMemberTypeDescription Member (int idx)
    {
        AssureMembers ();

        if (idx < 0 || idx >= mMembers)
            return null;

        return new UnoXInterfaceMemberTypeDescription (cmm_x_interface_type_description_get_member (mMembersHandle, idx));
    }

    [DllImport("climaker")]
    private static extern int cmm_x_interface_type_description_get_length (IntPtr handle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_type_description_get_base_type (IntPtr handle, int idx);

    [DllImport("climaker")]
    private static extern int cmm_x_interface_type_description_get_members (IntPtr handle, out IntPtr membersHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_type_description_get_member (IntPtr membersArrayHandle, int idx);
}

class UnoXInterfaceMemberTypeDescription : UnoXTypeDescription
{
    public UnoXInterfaceMemberTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public string MemberName
    {
        get
        {
            return Marshal.PtrToStringAnsi (cmm_x_interface_member_type_description_get_member_name (mHandle));
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_member_type_description_get_member_name (IntPtr handle);
}

class UnoXInterfaceAttributeTypeDescription : UnoXInterfaceMemberTypeDescription
{
    public UnoXInterfaceAttributeTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public UnoXTypeDescription Type
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_interface_attribute_type_description_get_type (mHandle));
        }
    }

    public bool IsBound
    {
        get
        {
            return cmm_x_interface_attribute_type_description_is_bound (mHandle);
        }
    }

    public bool IsReadOnly
    {
        get
        {
            return cmm_x_interface_attribute_type_description_is_read_only (mHandle);
        }
    }

    UnoXTypeDescription[] ExceptionTypes (IntPtr handle, int count)
    {
        int i;

        UnoXTypeDescription[] types = new UnoXTypeDescription [count];
        for (i = 0; i < count; i ++) {
            types [i] = UnoXTypeDescription.Create (cmm_x_interface_attribute_type_description_get_exception_type (handle, i));
        }

        return types;
    }

    public UnoXTypeDescription[] GetExceptionTypes
    {
        get
        {
            IntPtr handle;
            int count;

            count = cmm_x_interface_attribute_type_description_get_get_exception_types (mHandle, out handle);
            UnoXTypeDescription[] types = ExceptionTypes (handle, count);

            if (count > 0)
                cmm_x_interface_attribute_type_description_free_exception_types (handle, count);

            return types;
        }
    }

    public UnoXTypeDescription[] SetExceptionTypes
    {
        get
        {
            IntPtr handle;
            int count;

            count = cmm_x_interface_attribute_type_description_get_set_exception_types (mHandle, out handle);
            UnoXTypeDescription[] types = ExceptionTypes (handle, count);

            if (count > 0)
                cmm_x_interface_attribute_type_description_free_exception_types (handle, count);

            return types;
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_attribute_type_description_get_type (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_interface_attribute_type_description_is_bound (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_interface_attribute_type_description_is_read_only (IntPtr handle);

    [DllImport("climaker")]
    private static extern int cmm_x_interface_attribute_type_description_get_get_exception_types (IntPtr handle, out IntPtr exceptionTypesHandle);

    [DllImport("climaker")]
    private static extern int cmm_x_interface_attribute_type_description_get_set_exception_types (IntPtr handle, out IntPtr exceptionTypesHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_attribute_type_description_get_exception_type (IntPtr handle, int index);

    [DllImport("climaker")]
    private static extern void cmm_x_interface_attribute_type_description_free_exception_types (IntPtr handle, int count);
}

class UnoXInterfaceMethodTypeDescription : UnoXInterfaceMemberTypeDescription
{
    int mParameters = 0;
    IntPtr mParametersHandle = IntPtr.Zero;
    IntPtr mParametersArrayHandle = IntPtr.Zero;

    bool mGotExceptionTypes = false;
    int mExceptionTypes = 0;
    IntPtr mExceptionTypesHandle = IntPtr.Zero;
    IntPtr mExceptionTypesArrayHandle = IntPtr.Zero;

    public UnoXInterfaceMethodTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public UnoXTypeDescription ReturnType
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_interface_method_type_description_get_return_type (mHandle));
        }
    }

    public bool ReturnsStruct
    {
        get
        {
            return cmm_x_interface_method_type_description_returns_struct (mHandle);
        }
    }

    void AssureParameters ()
    {
        if (mParametersHandle == IntPtr.Zero)
            mParameters = cmm_x_interface_method_type_description_get_parameters (mHandle, out mParametersHandle, out mParametersArrayHandle);
    }

    public int Parameters
    {
        get
        {
            AssureParameters ();

            return mParameters;
        }
    }

    public UnoXMethodParameter Parameter (int idx)
    {
        AssureParameters ();

        if (idx < 0 || idx >= mParameters)
            return null;

        return new UnoXMethodParameter (cmm_x_interface_method_type_description_get_parameter (mParametersArrayHandle, idx));
    }

    void AssureExceptionTypes ()
    {
        if (!mGotExceptionTypes) {
            mExceptionTypes = cmm_x_interface_method_type_description_get_exception_types (mHandle, out mExceptionTypesHandle, out mExceptionTypesArrayHandle);
            mGotExceptionTypes = true;
        }
    }

    public int ExceptionTypes
    {
        get
        {
            AssureExceptionTypes ();

            return mExceptionTypes;
        }
    }

    public UnoXCompoundTypeDescription ExceptionType (int idx)
    {
        AssureExceptionTypes ();

        if (idx < 0 || idx >= mExceptionTypes)
            return null;

        return UnoXTypeDescription.Create (cmm_x_interface_method_type_description_get_exception_type (mExceptionTypesArrayHandle, idx)) as UnoXCompoundTypeDescription;
    }

    public bool IsOneway
    {
        get
        {
            return cmm_x_interface_method_type_description_is_oneway (mHandle);
        }
    }

    [DllImport("climaker")]
    private static extern int cmm_x_interface_method_type_description_get_parameters (IntPtr handle, out IntPtr parametersHandle, out IntPtr parametersArrayHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_method_type_description_get_parameter (IntPtr parametersArrayHandle, int idx);

    [DllImport("climaker")]
    private static extern int cmm_x_interface_method_type_description_get_exception_types (IntPtr handle, out IntPtr exceptionTypesHandle, out IntPtr exceptionTypesArrayHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_method_type_description_get_exception_type (IntPtr exceptionTypesArrayHandle, int idx);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_interface_method_type_description_get_return_type (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_interface_method_type_description_returns_struct (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_interface_method_type_description_is_oneway (IntPtr handle);
}

class UnoXCompoundTypeDescription : UnoXTypeDescription
{
    int mMemberNames = 0;
    IntPtr mMemberNamesHandle = IntPtr.Zero;

    int mMemberTypes = 0;
    IntPtr mMemberTypesHandle = IntPtr.Zero;

    public UnoXCompoundTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public UnoXTypeDescription BaseType
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_compound_type_description_get_base_type (mHandle));
        }
    }

    void AssureMemberNames ()
    {
        if (mMemberNamesHandle == IntPtr.Zero)
            mMemberNames = cmm_x_compound_type_description_get_member_names (mHandle, out mMemberNamesHandle);
    }

    public string MemberName (int idx)
    {
        AssureMemberNames ();

        if (idx < 0 || idx >= mMemberNames)
            return null;

        return Marshal.PtrToStringAnsi (cmm_x_compound_type_description_get_member_name (mMemberNamesHandle, idx));
    }

    public int MemberNames
    {
        get
        {
            AssureMemberNames ();

            return mMemberNames;
        }
    }

    void AssureMemberTypes ()
    {
        if (mMemberTypesHandle == IntPtr.Zero)
            mMemberTypes = cmm_x_compound_type_description_get_member_types (mHandle, out mMemberTypesHandle);
    }

    public UnoXTypeDescription MemberType (int idx)
    {
        AssureMemberTypes ();

        if (idx < 0 || idx >= mMemberTypes)
            return null;

        return UnoXTypeDescription.Create (cmm_x_compound_type_description_get_member_type (mMemberTypesHandle, idx));
    }

    public int MemberTypes
    {
        get
        {
            AssureMemberTypes ();

            return mMemberTypes;
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_compound_type_description_get_base_type (IntPtr handle);

    [DllImport("climaker")]
    private static extern int cmm_x_compound_type_description_get_member_names (IntPtr handle, out IntPtr memberNamesHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_compound_type_description_get_member_name (IntPtr handle, int idx);

    [DllImport("climaker")]
    private static extern int cmm_x_compound_type_description_get_member_types (IntPtr handle, out IntPtr memberTypesHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_compound_type_description_get_member_type (IntPtr handle, int idx);
}

class UnoXExceptionTypeDescription : UnoXCompoundTypeDescription
{
    public UnoXExceptionTypeDescription (IntPtr handle) : base (handle)
    {
    }
}

class UnoXStructTypeDescription : UnoXCompoundTypeDescription
{
    int mTypeArguments = 0;
    IntPtr mTypeArgumentsHandle = IntPtr.Zero;
    IntPtr mTypeArgumentsArrayHandle = IntPtr.Zero;

    int mTypeParameters = 0;
    IntPtr mTypeParametersHandle = IntPtr.Zero;

    public UnoXStructTypeDescription (IntPtr handle) : base (handle)
    {
    }

    void AssureTypeParameters ()
    {
        if (mTypeParametersHandle == IntPtr.Zero)
            mTypeParameters = cmm_x_struct_type_description_get_type_parameters (mHandle, out mTypeParametersHandle);
    }

    public string TypeParameter (int idx)
    {
        AssureTypeParameters ();

        if (idx < 0 || idx >= mTypeParameters)
            return null;

        return Marshal.PtrToStringAnsi (cmm_x_struct_type_description_get_type_parameter (mTypeParametersHandle, idx));
    }

    public int TypeParameters
    {
        get
        {
            AssureTypeParameters ();

            return mTypeParameters;
        }
    }

    void AssureTypeArguments ()
    {
        if (mTypeArgumentsHandle == IntPtr.Zero)
            mTypeArguments = cmm_x_struct_type_description_get_type_arguments (mHandle, out mTypeArgumentsHandle, out mTypeArgumentsArrayHandle);
    }

    public int TypeArguments
    {
        get
        {
            AssureTypeArguments ();

            return mTypeArguments;
        }
    }

    public UnoXTypeDescription TypeArgument (int idx)
    {
        AssureTypeArguments ();

        if (idx < 0 || idx >= mTypeArguments)
            return null;

        return UnoXTypeDescription.Create (cmm_x_struct_type_description_get_type_argument (mTypeArgumentsArrayHandle, idx));
    }

    [DllImport("climaker")]
    private static extern int cmm_x_struct_type_description_get_type_parameters (IntPtr handle, out IntPtr typeParametersHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_struct_type_description_get_type_parameter (IntPtr typeParametersHandle, int idx);

    [DllImport("climaker")]
    private static extern int cmm_x_struct_type_description_get_type_arguments (IntPtr handle, out IntPtr typeArgumentsHandle, out IntPtr typeArgumentsArrayHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_struct_type_description_get_type_argument (IntPtr typeArgumentsArrayHandle, int idx);
}

class UnoXMethodParameter : UnoObject
{
    public UnoXMethodParameter (IntPtr handle) : base (handle)
    {
    }

    public bool IsOut
    {
        get
        {
            return cmm_x_method_parameter_is_out (mHandle);
        }
    }

    public bool IsIn
    {
        get
        {
            return cmm_x_method_parameter_is_in (mHandle);
        }
    }

    public string Name
    {
        get
        {
            return Marshal.PtrToStringAnsi (cmm_x_method_parameter_name (mHandle));
        }
    }

    public int Position
    {
        get
        {
            return cmm_x_method_parameter_position (mHandle);
        }
    }

    public UnoXTypeDescription Type
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_method_parameter_type (mHandle));
        }
    }

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_method_parameter_is_out (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_method_parameter_is_in (IntPtr handle);

    [DllImport("climaker")]
    private static extern int cmm_x_method_parameter_position (IntPtr handle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_method_parameter_name (IntPtr handle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_method_parameter_type (IntPtr handle);
}

class UnoXParameter : UnoXMethodParameter
{
    public UnoXParameter (IntPtr handle) : base (handle)
    {
    }

    public bool IsRest
    {
        get
        {
            return cmm_x_parameter_is_rest (mHandle);
        }
    }

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_parameter_is_rest (IntPtr handle);
}

class UnoXIndirectTypeDescription : UnoXTypeDescription
{
    public UnoXIndirectTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public UnoXTypeDescription ReferencedType
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_indirect_type_description_get_referenced_type (mHandle));
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_indirect_type_description_get_referenced_type (IntPtr handle);
}

class UnoXServiceTypeDescription : UnoXTypeDescription
{
    int mConstructors = 0;
    IntPtr mConstructorsHandle = IntPtr.Zero;

    public UnoXServiceTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public bool IsSingleInterfaceBased
    {
        get
        {
            return cmm_x_service_type_description_is_single_interface_based (mHandle);
        }
    }

    public UnoXTypeDescription Interface
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_service_type_description_get_interface (mHandle));
        }
    }

    void AssureConstructors ()
    {
        if (mConstructorsHandle == IntPtr.Zero)
            mConstructors = cmm_x_service_type_description_get_constructors (mHandle, out mConstructorsHandle);
    }

    public int Constructors
    {
        get
        {
            AssureConstructors ();

            return mConstructors;
        }
    }

    public UnoXServiceConstructorDescription Constructor (int idx)
    {
        AssureConstructors ();

        if (idx < 0 || idx >= mConstructors)
            return null;

        return new UnoXServiceConstructorDescription (cmm_x_service_type_description_get_constructor (mConstructorsHandle, idx));
    }

    [DllImport("climaker")]
    private static extern int cmm_x_service_type_description_get_constructors (IntPtr handle, out IntPtr constructorssHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_service_type_description_get_constructor (IntPtr constructorsHandle, int idx);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_service_type_description_is_single_interface_based (IntPtr handle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_service_type_description_get_interface (IntPtr handle);
}

class UnoXSingletonTypeDescription : UnoXTypeDescription
{
    public UnoXSingletonTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public bool IsInterfaceBased
    {
        get
        {
            return cmm_x_singleton_type_description_is_interface_based (mHandle);
        }
    }

    public UnoXTypeDescription Interface
    {
        get
        {
            return UnoXTypeDescription.Create (cmm_x_singleton_type_description_get_interface (mHandle));
        }
    }

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_singleton_type_description_get_interface (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_singleton_type_description_is_interface_based (IntPtr handle);
}

class UnoXConstantTypeDescription : UnoXTypeDescription
{
    public UnoXConstantTypeDescription (IntPtr handle) : base (handle)
    {
    }

    public object ConstantValue
    {
        get
        {
            char tChar;
            bool tBool;
            byte tByte;
            Int16 tInt16;
            System.UInt16 tUInt16;
            Int32 tInt32;
            System.UInt32 tUInt32;
            Int64 tInt64;
            System.UInt64 tUInt64;
            System.Single tFloat;
            double tDouble;

            UnoTypeClass tc = cmm_any_to_cli_constant (mHandle,
                                                       out tChar, out tBool,
                                                       out tByte,
                                                       out tInt16, out tUInt16,
                                                       out tInt32, out tUInt32,
                                                       out tInt64, out tUInt64,
                                                       out tFloat, out tDouble);
            switch (tc) {
            case UnoTypeClass.Char:
                return tChar;
            case UnoTypeClass.Boolean:
                return tBool;
            case UnoTypeClass.Byte:
                return tByte;
            case UnoTypeClass.Short:
                return tInt16;
            case UnoTypeClass.UnsignedShort:
                return tUInt16;
            case UnoTypeClass.Long:
                return tInt32;
            case UnoTypeClass.UnsignedLong:
                return tUInt32;
            case UnoTypeClass.Hyper:
                return tInt64;
            case UnoTypeClass.UnsignedHyper:
                return tUInt64;
            case UnoTypeClass.Float:
                return tFloat;
            case UnoTypeClass.Double:
                return tDouble;
            }

            throw new Exception (String.Format ("Unexpected type {0} for XConstantTypeDescription", tc));
        }
    }

    [DllImport("climaker")]
    private static extern UnoTypeClass cmm_any_to_cli_constant (IntPtr handle,
                                out char tChar, out bool tBool,
                                out byte tByte,
                                out Int16 tInt16, out System.UInt16 tUInt16,
                                out Int32 tInt32, out System.UInt32 tUInt32,
                                out Int64 tInt64, out System.UInt64 tUInt64,
                                out System.Single tFloat, out double tDouble);
}

class UnoXConstantsTypeDescription : UnoXTypeDescription
{
    int mConstants = 0;
    IntPtr mConstantsHandle = IntPtr.Zero;

    public UnoXConstantsTypeDescription (IntPtr handle) : base (handle)
    {
    }

    void AssureConstants ()
    {
        if (mConstantsHandle == IntPtr.Zero)
            mConstants = cmm_x_constants_type_description_get_constants (mHandle, out mConstantsHandle);
    }

    public int Constants
    {
        get
        {
            AssureConstants ();

            return mConstants;
        }
    }

    public UnoXConstantTypeDescription Constant (int idx)
    {
        AssureConstants ();

        if (idx < 0 || idx >= mConstants)
            return null;

        return UnoXTypeDescription.Create (cmm_x_constants_type_description_get_constant (mConstantsHandle, idx)) as UnoXConstantTypeDescription;
    }

    [DllImport("climaker")]
    private static extern int cmm_x_constants_type_description_get_constants (IntPtr handle, out IntPtr constantssHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_constants_type_description_get_constant (IntPtr constantsHandle, int idx);
}

class UnoXServiceConstructorDescription : UnoObject
{
    int mParameters = 0;
    IntPtr mParametersHandle = IntPtr.Zero;

    public UnoXServiceConstructorDescription (IntPtr handle) : base (handle)
    {
    }

    void AssureParameters ()
    {
        if (mParametersHandle == IntPtr.Zero)
            mParameters = cmm_x_service_constructor_description_get_parameters (mHandle, out mParametersHandle);
    }

    public int Parameters
    {
        get
        {
            AssureParameters ();

            return mParameters;
        }
    }

    public UnoXParameter Parameter (int idx)
    {
        AssureParameters ();

        if (idx < 0 || idx >= mParameters)
            return null;

        return new UnoXParameter (cmm_x_service_constructor_description_get_parameter (mParametersHandle, idx));
    }

    public string Name
    {
        get
        {
            return Marshal.PtrToStringAnsi (cmm_x_service_constructor_description_get_name (mHandle));
        }
    }

    public bool IsDefault
    {
        get
        {
            return cmm_x_service_constructor_description_is_default_constructor (mHandle);
        }
    }

    public UnoXTypeDescription[] ExceptionTypes
    {
        get
        {
            IntPtr handle;
            int i, count;

            count = cmm_x_service_constructor_description_get_exception_types (mHandle, out handle);
            UnoXTypeDescription[] types = new UnoXTypeDescription [count];

            for (i = 0; i < count; i ++) {
                types [i] = UnoXTypeDescription.Create (cmm_x_service_constructor_description_get_exception_type (handle, i));
            }

            if (count > 0)
                cmm_x_service_constructor_description_free_exception_types (handle, count);

            return types;
        }
    }

    [DllImport("climaker")]
    private static extern int cmm_x_service_constructor_description_get_parameters (IntPtr handle, out IntPtr parameterssHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_service_constructor_description_get_parameter (IntPtr parametersHandle, int idx);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_service_constructor_description_get_name (IntPtr handle);

    [DllImport("climaker")][return:MarshalAs(UnmanagedType.I1)]
    private static extern bool cmm_x_service_constructor_description_is_default_constructor (IntPtr handle);

    [DllImport("climaker")]
    private static extern int cmm_x_service_constructor_description_get_exception_types (IntPtr handle, out IntPtr exceptionTypesHandle);

    [DllImport("climaker")]
    private static extern IntPtr cmm_x_service_constructor_description_get_exception_type (IntPtr handle, int index);

    [DllImport("climaker")]
    private static extern void cmm_x_service_constructor_description_free_exception_types (IntPtr handle, int count);
}


public class MainClass
{
    class OptionInfo
    {
        public string mName;
        public char mShortOption;
        public bool mHasArgument;

        public OptionInfo (string name, char shortOption, bool hasArgument)
        {
            mName = name;
            mShortOption = shortOption;
            mHasArgument = hasArgument;
        }
    }

    static OptionInfo[] sOptionInfos = {
        new OptionInfo ("out", 'O', true ),
        new OptionInfo ("types", 'T', true),
        new OptionInfo ("extra", 'X', true),
        new OptionInfo ("reference", 'r', true),
        new OptionInfo ("keyfile", 'k', true),
        new OptionInfo ("delaySign", 'd', true),
        new OptionInfo ("assembly-version", '\0', true),
        new OptionInfo ("assembly-description", '\0', true),
        new OptionInfo ("assembly-product", '\0', true),
        new OptionInfo ("assembly-company", '\0', true),
        new OptionInfo ("assembly-copyright", '\0', true),
        new OptionInfo ("assembly-trademark", '\0', true),
        new OptionInfo ("verbose", 'v', false),
        new OptionInfo ("help", 'h', false)
    };

    static string sUsingText =
        "\n" +
        "using: climaker <switches> [registry-file-1 registry-file-2 ...]\n" +
        "\n" +
        "switches:\n" +
        " -O, --out <output-file>       output assembly file;\n" +
        "                               defaults to cli_unotypes.dll if more than one\n" +
        "                               registry-file is given, else <registry-file>.dll\n" +
        " -T, --types                   types to be generated (if none is given,\n" +
        "   <type1[;type2;...]>         then all types of given registries are emitted\n" +
        " -X, --extra <rdb-file>        additional rdb to saturate referenced types in\n" +
        "                               given registry file(s); these types will not be\n" +
        "                               emitted into the output assembly file\n" +
        " -r, --reference               reference metadata from assembly file\n" +
        "   <assembly-file>\n" +
        " -k, --keyfile                 keyfile needed for strong name\n" +
        " --assembly-version <version>  sets assembly version\n" +
        " --assembly-description <text> sets assembly description text\n" +
        " --assembly-product <text>     sets assembly product name\n" +
        " --assembly-company <text>     sets assembly company\n" +
        " --assembly-copyright <text>   sets assembly copyright\n" +
        " --assembly-trademark <text>   sets assembly trademark\n" +
        " -v, --verbose                 verbose output to stdout\n" +
        " -h, --help                    this message\n" +
        "\n" +
        "example: climaker --out cli_mytypes.dll \\\n" +
        "                  --reference cli_uretypes.dll \\\n" +
        "                  --extra types.rdb \\\n" +
        "                  mytypes.rdb\n" +
        "\n";

    static OptionInfo GetOptionInfo (string opt)
    {
        return GetOptionInfo (opt, '\0');
    }

    static OptionInfo GetOptionInfo (string opt, char shortOpt)
    {
        int pos;

        for (pos = 0; pos < sOptionInfos.Length; pos ++) {
            if (opt.Length > 0) {
                if (opt.Equals (sOptionInfos [pos].mName))
                    return sOptionInfos [pos];
            } else {
                if (sOptionInfos [pos].mShortOption == shortOpt)
                    return sOptionInfos [pos];
            }
        }

        return null;
    }

    static bool IsOption (OptionInfo optionInfo, ref int pIndex)
    {
        if (sArgs.Length <= pIndex)
            return false;
        string arg = sArgs [pIndex];

        if (arg.Length < 2 || arg [0] != '-')
            return false;

        if (arg.Length == 2 && arg[ 1 ] == optionInfo.mShortOption) {
            pIndex ++;

            return true;
        }

        if (arg [1] == '-' && arg.Substring (2).Equals (optionInfo.mName)) {
            pIndex ++;

            return true;
        }

        return false;
    }

    static bool ReadOption (ref bool flag, OptionInfo optionInfo, ref int pIndex)
    {
        bool ret = IsOption (optionInfo, ref pIndex);
        if (ret)
            flag = true;

        return ret;
    }

    static bool ReadArgument (ref string pValue, OptionInfo optionInfo, ref int pIndex)
    {
        if (IsOption (optionInfo, ref pIndex)) {
            if (pIndex < sArgs.Length) {
                pValue = sArgs [pIndex];
                pIndex ++;

                return true;
            }
            pIndex --;
        }

        return false;
    }

    static string[] sArgs;

    public static int Main (string[] args)
    {
        if (args.Length <= 0) {
            Console.Write (sUsingText);
            return 0;
        }

        int ret = 0;

        try {
            sArgs = args;
            Config config = new Config ();

            OptionInfo infoHelp = GetOptionInfo ("help");
            OptionInfo infoVerbose = GetOptionInfo ("verbose");
            OptionInfo infoOut = GetOptionInfo ("out");
            OptionInfo infoTypes = GetOptionInfo ("types");
            OptionInfo infoReference = GetOptionInfo ("reference");
            OptionInfo infoExtra = GetOptionInfo ("extra");
            OptionInfo infoKeyfile = GetOptionInfo ("keyfile");
            OptionInfo infoDelaysign = GetOptionInfo ("delaySign");
            OptionInfo infoVersion = GetOptionInfo ("assembly-version");
            OptionInfo infoProduct = GetOptionInfo ("assembly-product");
            OptionInfo infoDescription = GetOptionInfo ("assembly-description");
            OptionInfo infoCompany = GetOptionInfo ("assembly-company");
            OptionInfo infoCopyright = GetOptionInfo ("assembly-copyright");
            OptionInfo infoTrademark = GetOptionInfo ("assembly-trademark");

            int pos;
            string cmdArg = null;

            for (pos = 0; pos < sArgs.Length; ) {
                // options
                if (IsOption (infoHelp, ref pos )) {
                    Console.Write (sUsingText);
                    return 0;
                } else if (ReadArgument (ref cmdArg, infoTypes, ref pos )) {
                    config.mExplicitTypes.AddRange (cmdArg.Split (new char[] {';'}));
                } else if (ReadArgument (ref cmdArg, infoExtra, ref pos )) {
                    config.mExtraRegistries.Add (cmdArg);
                } else if (ReadArgument (ref cmdArg, infoReference, ref pos )) {
                    config.mExtraAssemblies.Add (cmdArg);
                } else if (!ReadOption (ref config.mVerbose, infoVerbose, ref pos) &&
                           !ReadArgument (ref config.mOutput, infoOut, ref pos) &&
                           !ReadArgument (ref config.mVersion, infoVersion, ref pos) &&
                           !ReadArgument (ref config.mDescription, infoDescription, ref pos) &&
                           !ReadArgument (ref config.mProduct, infoProduct, ref pos) &&
                           !ReadArgument (ref config.mCompany, infoCompany, ref pos) &&
                           !ReadArgument (ref config.mCopyright, infoCopyright, ref pos) &&
                           !ReadArgument (ref config.mTrademark, infoTrademark, ref pos) &&
                           !ReadArgument (ref config.mKeyfile, infoKeyfile, ref pos) &&
                           !ReadArgument (ref config.mDelaySign, infoDelaysign, ref pos)) {
                    cmdArg = args [pos];
                    pos ++;
                    cmdArg = cmdArg.Trim();

                    if (cmdArg.Length > 0) {
                        if (cmdArg [0] == '-') { // is option
                            OptionInfo optionInfo = null;

                            if (cmdArg.Length > 2 && cmdArg [1] == '-') {
                                // long option
                                optionInfo = GetOptionInfo (cmdArg.Substring (2));
                            } else if (cmdArg.Length == 2 && cmdArg [1] != '-') {
                                // short option
                                optionInfo = GetOptionInfo ("", cmdArg [1]);
                            }

                            if (optionInfo == null) {
                                Console.WriteLine ("unknown option " + cmdArg + "!  Use climaker --help\nto print all options.");
                                return 1;
                            } else {
                                Console.WriteLine ("unhandled valid option?! " + cmdArg);
                                if (optionInfo.mHasArgument)
                                    pos ++;
                            }
                        } else {
                            config.mMandatoryRegistries.Add (cmdArg);
                        }
                    }
                }
            }

            CliMaker cliMaker = new CliMaker (config);

            cliMaker.OpenRegistries ();
            cliMaker.PrepareAssembly ();
            cliMaker.Emit ();
            cliMaker.Save ();
        } catch (Exception e) {
            Console.WriteLine ("\n> error: " + e + "\n> dying abnormally...");
            ret = 1;
        }

        return ret;
    }
}
