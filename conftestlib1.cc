template<typename T> struct S1 { virtual ~S1() {} virtual void f() {} };
struct S2: S1<int> { virtual ~S2(); };
S2::~S2() {}
