#include "sal/types.h"
#define T1(x) do { x; } while (0)
#define T2(x) do { x; } while (sal_False)
#define T3(x) do { x; } while (false)
#define T4(x, y) do { x; } while (y)
#define T5(x) T4(x, 0)
#define T6(x) T4(x, sal_False)
#define T7(x) T4(x, false)
