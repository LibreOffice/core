#include "CodeFragmentTypes.r"

resource 'cfrg' (0) {
	{
		kPowerPC,							/* instruction set architecture */
		kFullLib,							/* base-level library */
		kNoVersionNum,						/* no implementation version number */
		kNoVersionNum,						/* no definition version number */
		kDefaultStackSize,					/* use default stack size */
		kNoAppSubFolder,					/* no library directory */
		kIsLib,								/* fragment is an application */
		kOnDiskFlat,						/* fragment is on disk */
		kZeroOffset,						/* fragment starts at fork start */
		kWholeFork, 						/* fragment occupies entire fork */
		"Tools.pef"							/* name of the application */
	}
};
