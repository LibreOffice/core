
Necessary enty in scp2, for example cli_types (gac + program\assembly) + policy assembly
========================

#if defined WNT && (defined M1300 || defined M1310) // currently the only cli platform

File gid_File_Lib_Cli_Types
    TXT_FILE_BODY;
    Styles = (PACKED, PATCH);
    Name = "cli_types.dll";
    Dir = gid_Dir_Program_Assembly;
End

#endif

#if defined WNT && (defined M1300 || defined M1310) // currently the only cli platform

File gid_File_Lib_Cli_Types_Assembly
    TXT_FILE_BODY;
    Styles = (PACKED, ASSEMBLY, PATCH, DONTRENAMEINPATCH);
    Name = "cli_types.dll";
    Dir = gid_Dir_Program;
    Assemblyname = "cli_types";
    PublicKeyToken = "ce2cb7e279207b9e";
    Version = "1.1.1.0";
    Culture = "neutral";
    Attributes = "0";
End

#endif

#if defined WNT && (defined M1300 || defined M1310) // currently the only cli platform

File gid_File_Lib_Policy_Cli_Types_Assembly
    TXT_FILE_BODY;
    Styles = (PACKED, ASSEMBLY, PATCH, DONTRENAMEINPATCH);
    Name = "policy.1.1.cli_types.dll";
    Dir = gid_Dir_Program;
    Assemblyname = "policy.1.1.cli_types";
    PublicKeyToken = "ce2cb7e279207b9e";
    Version = "1.0.0.0";
    Culture = "neutral";
    Attributes = "0";
End

#endif

#if defined WNT && (defined M1300 || defined M1310) // currently the only cli platform

File gid_File_Lib_Policy_Cli_Types_Config
    TXT_FILE_BODY;
    Styles = (PACKED, ASSIGNCOMPOMENT, PATCH, DONTRENAMEINPATCH);
    Name = "cli_types.config";
    Dir = gid_Dir_Program;
    AssignComponent = gid_File_Lib_Policy_Cli_Types_Assembly;
End




Versions of assemblies
==========================
SRC680 m122	
        cli_types.dll 1.1.0.0   (the version was actually changed some time before that milestone)
	no policy assembly
	cli_cppuhelper 1.0.0.0
	no policy assembly
	cli_basetypes.dll 1.0.0.0
	no policy assembly
	cli_ure.dll 1.0.0.0
	no policy assembly

		

