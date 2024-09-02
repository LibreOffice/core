/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using com.sun.star.uno.helper;

namespace com.sun.star.uno.native
{
    internal class Marshaller
    {
        private readonly NetEnvironment _env;
        public Marshaller(NetEnvironment env) => _env = env;

        public unsafe void MarshalObject(Type type, object src, InteropValue* value, bool destructValue)
            => MarshalObject(type, src, (void*)value, destructValue);

        public unsafe void UnmarshalObject(Type type, ref object dst, InteropValue* value, bool assignObject)
            => UnmarshalObject(type, ref dst, (void*)value, assignObject);

        private unsafe void MarshalObject(Type type, object source, void* value, bool destructValue)
        {
            switch (Type.GetTypeCode(type))
            {
                case TypeCode.Boolean:
                    *(byte*)value = (byte)((bool)source ? 1 : 0);
                    break;
                case TypeCode.SByte:
                    *(sbyte*)value = (sbyte)source;
                    break;
                case TypeCode.Char:
                    *(char*)value = (char)source;
                    break;
                case TypeCode.Int16:
                    *(short*)value = (short)source;
                    break;
                case TypeCode.UInt16:
                    *(ushort*)value = (ushort)source;
                    break;
                case TypeCode.Int32:
                    *(int*)value = (int)source;
                    break;
                case TypeCode.UInt32:
                    *(uint*)value = (uint)source;
                    break;
                case TypeCode.Int64:
                    *(long*)value = (long)source;
                    break;
                case TypeCode.UInt64:
                    *(ulong*)value = (ulong)source;
                    break;
                case TypeCode.Single:
                    *(float*)value = (float)source;
                    break;
                case TypeCode.Double:
                    *(double*)value = (double)source;
                    break;
                case TypeCode.String:
                    {
                        IntPtr* valueStr = (IntPtr*)value;
                        string netStr = (string)source;

                        if (destructValue && *valueStr != IntPtr.Zero)
                            InteropMethods.freeMemory(*valueStr);

                        *valueStr = MarshalString(netStr);
                        break;
                    }
                default:
                    if (type == typeof(void))
                    {
                        *(IntPtr*)value = IntPtr.Zero;
                    }
                    else if (type == typeof(Type))
                    {
                        IntPtr* valueType = (IntPtr*)value;
                        Type netType = (Type)source;

                        if (destructValue && *valueType != IntPtr.Zero)
                            InteropMethods.freeMemory(*valueType);
                        *valueType = MarshalString(netType.FullName);
                    }
                    else if (type == typeof(Any))
                    {
                        InteropValue.Any* valueAny = (InteropValue.Any*)value;
                        Any netAny = (Any)source;

                        switch (Type.GetTypeCode(netAny.Type))
                        {
                            case TypeCode.Boolean:
                            case TypeCode.SByte:
                            case TypeCode.Char:
                            case TypeCode.Int16:
                            case TypeCode.UInt16:
                            case TypeCode.Int32:
                            case TypeCode.UInt32:
                            case TypeCode.Single:
                                MarshalObject(netAny.Type, netAny.Value, (void*)&valueAny->data, destructValue);
                                break;
                            case TypeCode.Int64:
                            case TypeCode.UInt64:
                            case TypeCode.Double:
                                {
                                    int size = SizeOf(netAny.Type);
                                    if (size <= IntPtr.Size)
                                        MarshalObject(netAny.Type, netAny.Value, (void*)&valueAny->data, destructValue);
                                    else
                                    {
                                        IntPtr mem = InteropMethods.allocateMemory(size);
                                        MarshalObject(netAny.Type, netAny.Value, (void*)mem, false);
                                        if (destructValue)
                                            InteropMethods.freeMemory(valueAny->data);
                                        valueAny->data = mem;
                                    }
                                    break;
                                }
                            default:
                                if (netAny.Type == typeof(Any) || netAny.Type.IsArray)
                                {
                                    IntPtr mem = InteropMethods.allocateMemory(SizeOf(netAny.Type));
                                    MarshalObject(netAny.Type, netAny.Value, (void*)mem, false);
                                    if (destructValue)
                                        InteropMethods.freeMemory(valueAny->data);
                                    valueAny->data = mem;
                                }
                                else
                                {
                                    MarshalObject(netAny.Type, netAny.Value, (void*)&valueAny->data, destructValue);
                                }
                                break;
                        }

                        MarshalObject(typeof(Type), netAny.Type, (void*)&valueAny->type, destructValue);
                    }
                    else if (type.IsEnum)
                    {
                        *(int*)value = (int)source;
                    }
                    else if (type.IsArray)
                    {
                        InteropValue.Sequence* valueSeq = (InteropValue.Sequence*)value;
                        Array netArray = (Array)source;

                        Type elemType = type.GetElementType();
                        int elemSize = SizeOf(elemType);

                        int memSize = elemSize * netArray.Length;
                        IntPtr mem = InteropMethods.allocateMemory(memSize);

                        switch (Type.GetTypeCode(elemType))
                        {
                            case TypeCode.Boolean:
                                fixed (void* src = (bool[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.SByte:
                                fixed (void* src = (sbyte[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Char:
                                fixed (void* src = (char[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Int16:
                                fixed (void* src = (short[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.UInt16:
                                fixed (void* src = (ushort[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Int32:
                                fixed (void* src = (int[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.UInt32:
                                fixed (void* src = (uint[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Int64:
                                fixed (void* src = (long[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.UInt64:
                                fixed (void* src = (ulong[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Single:
                                fixed (void* src = (float[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            case TypeCode.Double:
                                fixed (void* src = (double[])netArray)
                                    Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                break;
                            default:
                                if (elemType.IsEnum)
                                {
                                    fixed (void* src = (int[])netArray)
                                        Buffer.MemoryCopy(src, (void*)mem, memSize, memSize);
                                }
                                else
                                {
                                    for (int i = 0; i < netArray.Length; i++)
                                        MarshalObject(elemType, netArray.GetValue(i), (void*)(mem + elemSize * i), destructValue);
                                }
                                break;
                        }

                        if (destructValue && valueSeq->data != IntPtr.Zero)
                            InteropMethods.freeMemory(valueSeq->data);

                        valueSeq->data = mem;
                        valueSeq->length = netArray.Length;
                    }
                    else if (typeof(IQueryInterface).IsAssignableFrom(type))
                    {
                        *(IntPtr*)value = source is NativeUnoProxy nup
                            ? _env.LookupInterface(nup.Oid, nup.Type)
                            : _env.RegisterLocal(source, type);
                    }
                    else
                    {
                        IntPtr* valueStruct = (IntPtr*)value;

                        (Action<object, object[]> dtor, Type[] fieldTypes) = StructHelper.GetDeconstructor(type);
                        object[] fields = new object[fieldTypes.Length];
                        dtor.Invoke(source, fields);

                        IntPtr mem = InteropMethods.allocateMemory(fieldTypes.Sum(SizeOf));

                        int offset = 0;
                        for (int i = 0; i < fieldTypes.Length; i++)
                        {
                            MarshalObject(fieldTypes[i], fields[i], (void*)(mem + offset), destructValue);
                            offset += SizeOf(fieldTypes[i]);
                        }

                        if (destructValue && *valueStruct != IntPtr.Zero)
                            InteropMethods.freeMemory(*valueStruct);

                        *valueStruct = mem;
                    }
                    break;
            }
        }

        private unsafe void UnmarshalObject(Type type, ref object target, void* value, bool assignObject)
        {
            switch (Type.GetTypeCode(type))
            {
                case TypeCode.Boolean:
                    if (assignObject)
                        target = *(byte*)value != 0;
                    break;
                case TypeCode.SByte:
                    if (assignObject)
                        target = *(byte*)value;
                    break;
                case TypeCode.Char:
                    if (assignObject)
                        target = *(char*)value;
                    break;
                case TypeCode.Int16:
                    if (assignObject)
                        target = *(short*)value;
                    break;
                case TypeCode.UInt16:
                    if (assignObject)
                        target = *(ushort*)value;
                    break;
                case TypeCode.Int32:
                    if (assignObject)
                        target = *(int*)value;
                    break;
                case TypeCode.UInt32:
                    if (assignObject)
                        target = *(uint*)value;
                    break;
                case TypeCode.Int64:
                    if (assignObject)
                        target = *(long*)value;
                    break;
                case TypeCode.UInt64:
                    if (assignObject)
                        target = *(ulong*)value;
                    break;
                case TypeCode.Single:
                    if (assignObject)
                        target = *(float*)value;
                    break;
                case TypeCode.Double:
                    if (assignObject)
                        target = *(double*)value;
                    break;
                case TypeCode.String:
                    {
                        IntPtr valueStr = *(IntPtr*)value;
                        if (assignObject)
                            target = UnmarshalString(valueStr);
                        else
                            InteropMethods.freeMemory(valueStr);
                    }
                    break;
                default:
                    if (type == typeof(void))
                    {
                        if (assignObject)
                            target = null;
                    }
                    else if (type == typeof(Type))
                    {
                        IntPtr valueType = *(IntPtr*)value;
                        if (assignObject)
                            target = TypeHelper.ParseType(UnmarshalString(valueType));
                        else
                            InteropMethods.freeMemory(valueType);
                    }
                    else if (type == typeof(Any))
                    {
                        InteropValue.Any* valueAny = (InteropValue.Any*)value;

                        object contents = null;
                        Type containedType = TypeHelper.ParseType(UnmarshalString(valueAny->type));

                        if (assignObject)
                        {
                            switch (Type.GetTypeCode(containedType))
                            {
                                case TypeCode.Boolean:
                                case TypeCode.SByte:
                                case TypeCode.Char:
                                case TypeCode.Int16:
                                case TypeCode.UInt16:
                                case TypeCode.Int32:
                                case TypeCode.UInt32:
                                case TypeCode.Single:
                                    UnmarshalObject(containedType, ref contents, (void*)&valueAny->data, true);
                                    break;
                                case TypeCode.Int64:
                                case TypeCode.UInt64:
                                case TypeCode.Double:
                                    {
                                        int size = SizeOf(containedType);
                                        if (size <= IntPtr.Size)
                                        {
                                            UnmarshalObject(containedType, ref contents, (void*)&valueAny->data, true);
                                        }
                                        else
                                        {
                                            UnmarshalObject(containedType, ref contents, (void*)valueAny->data, true);
                                            InteropMethods.freeMemory(valueAny->data);
                                        }
                                        break;
                                    }
                                default:
                                    if (containedType == typeof(Any) || containedType.IsArray)
                                    {
                                        UnmarshalObject(containedType, ref contents, (void*)valueAny->data, true);
                                        InteropMethods.freeMemory(valueAny->data);
                                    }
                                    else
                                    {
                                        UnmarshalObject(containedType, ref contents, (void*)&valueAny->data, true);
                                    }
                                    break;
                            }
                            target = new Any(containedType, contents);
                        }
                        else
                        {
                            switch (Type.GetTypeCode(containedType))
                            {
                                case TypeCode.Boolean:
                                case TypeCode.SByte:
                                case TypeCode.Char:
                                case TypeCode.Int16:
                                case TypeCode.UInt16:
                                case TypeCode.Int32:
                                case TypeCode.UInt32:
                                case TypeCode.Single:
                                    break;
                                case TypeCode.Int64:
                                case TypeCode.UInt64:
                                case TypeCode.Double:
                                    if (SizeOf(containedType) > IntPtr.Size)
                                        InteropMethods.freeMemory(valueAny->data);
                                    break;
                                default:
                                    if (containedType == typeof(Any) || containedType.IsArray)
                                    {
                                        UnmarshalObject(containedType, ref contents, (void*)valueAny->data, false);
                                        InteropMethods.freeMemory(valueAny->data);
                                    }
                                    else
                                    {
                                        UnmarshalObject(containedType, ref contents, (void*)&valueAny->data, false);
                                    }
                                    break;
                            }
                        }
                    }
                    else if (type.IsEnum)
                    {
                        target = *(int*)value;
                    }
                    else if (type.IsArray)
                    {
                        InteropValue.Sequence* valueSeq = (InteropValue.Sequence*)value;

                        Type elemType = type.GetElementType();
                        int elemSize = SizeOf(elemType);

                        if (assignObject)
                        {
                            int memSize = elemSize * valueSeq->length;
                            Array netArray = Array.CreateInstance(elemType, valueSeq->length);

                            switch (Type.GetTypeCode(elemType))
                            {
                                case TypeCode.Boolean:
                                    fixed (void* dst = (bool[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.SByte:
                                    fixed (void* dst = (sbyte[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Char:
                                    fixed (void* dst = (char[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Int16:
                                    fixed (void* dst = (short[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.UInt16:
                                    fixed (void* dst = (ushort[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Int32:
                                    fixed (void* dst = (int[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.UInt32:
                                    fixed (void* dst = (uint[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Int64:
                                    fixed (void* dst = (long[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.UInt64:
                                    fixed (void* dst = (ulong[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Single:
                                    fixed (void* dst = (float[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                case TypeCode.Double:
                                    fixed (void* dst = (double[])netArray)
                                        Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    break;
                                default:
                                    if (elemType.IsEnum)
                                    {
                                        fixed (void* dst = (int[])netArray)
                                            Buffer.MemoryCopy((void*)valueSeq->data, dst, memSize, memSize);
                                    }
                                    else
                                    {
                                        object inner = null;
                                        for (int i = 0; i < netArray.Length; i++)
                                        {
                                            UnmarshalObject(elemType, ref inner, (void*)(valueSeq->data + elemSize * i), true);
                                            netArray.SetValue(inner, i);
                                        }
                                    }
                                    break;
                            }
                            target = netArray;
                        }
                        else
                        {
                            switch (Type.GetTypeCode(elemType))
                            {
                                case TypeCode.Boolean:
                                case TypeCode.SByte:
                                case TypeCode.Char:
                                case TypeCode.Int16:
                                case TypeCode.UInt16:
                                case TypeCode.Int32:
                                case TypeCode.UInt32:
                                case TypeCode.Int64:
                                case TypeCode.UInt64:
                                case TypeCode.Single:
                                case TypeCode.Double:
                                    break;
                                default:
                                    if (elemType.IsEnum || typeof(IQueryInterface).IsAssignableFrom(type))
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        object dummy = null;
                                        for (int i = 0; i < valueSeq->length; i++)
                                            UnmarshalObject(elemType, ref dummy, (void*)(valueSeq->data + elemSize * i), false);
                                    }
                                    break;
                            }
                        }

                        InteropMethods.freeMemory(valueSeq->data);
                    }
                    else if (typeof(IQueryInterface).IsAssignableFrom(type))
                    {
                        if (assignObject)
                            target = _env.GetRegisteredObject(*(IntPtr*)value);
                    }
                    else
                    {
                        IntPtr valueStruct = *(IntPtr*)value;

                        (Func<object[], object> ctor, Type[] paramTypes) = StructHelper.GetConstructor(type);
                        if (assignObject)
                        {
                            object[] args = new object[paramTypes.Length];
                            int offset = 0;
                            for (int i = 0; i < paramTypes.Length; i++)
                            {
                                UnmarshalObject(paramTypes[i], ref args[i], (void*)(valueStruct + offset), true);
                                offset += SizeOf(paramTypes[i]);
                            }
                            target = ctor(args);
                        }
                        else
                        {
                            object dummy = null;
                            int offset = 0;
                            for (int i = 0; i < paramTypes.Length; i++)
                            {
                                UnmarshalObject(paramTypes[i], ref dummy, (void*)(valueStruct + offset), false);
                                offset += SizeOf(paramTypes[i]);
                            }
                        }

                        InteropMethods.freeMemory(valueStruct);
                    }
                    break;
            }
        }

        private unsafe IntPtr MarshalString(string str)
        {
            char* buffer = (char*)InteropMethods.allocateMemory((str.Length + 1) * sizeof(char));
            fixed (char* data = str)
            {
                Buffer.MemoryCopy(data, buffer, str.Length * sizeof(char), str.Length * sizeof(char));
                buffer[str.Length] = '\0';
            }
            return (IntPtr)buffer;
        }

        private string UnmarshalString(IntPtr ptr)
        {
            string s = Marshal.PtrToStringUni(ptr);
            InteropMethods.freeMemory(ptr);
            return s;
        }

        private int SizeOf(Type type)
        {
            switch (Type.GetTypeCode(type))
            {
                case TypeCode.Boolean: return sizeof(bool);
                case TypeCode.SByte: return sizeof(sbyte);
                case TypeCode.Char: return sizeof(char);
                case TypeCode.Int16: return sizeof(short);
                case TypeCode.UInt16: return sizeof(ushort);
                case TypeCode.Int32: return sizeof(int);
                case TypeCode.UInt32: return sizeof(uint);
                case TypeCode.Int64: return sizeof(long);
                case TypeCode.UInt64: return sizeof(ulong);
                case TypeCode.Single: return sizeof(float);
                case TypeCode.Double: return sizeof(double);
            }
            if (type.IsEnum)
                return sizeof(int);
            else if (type.IsArray)
                return IntPtr.Size + sizeof(int);
            else if (type == typeof(Any))
                return IntPtr.Size * 2;
            return IntPtr.Size;
        }
    }
}
