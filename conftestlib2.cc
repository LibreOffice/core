template<typename T> struct S1 { virtual ~S1() {} virtual void f() {} };
struct S2: S1<int> { virtual ~S2(); };
struct S3: S2 { virtual ~S3(); }; S3::~S3() {}
