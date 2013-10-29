const char* publicFunc =
 "int isNan(double a) { return a != a; }\n"
 "double fsum(double a, double b) { return isNan(a)?b:a+b; }\n"
 "double fsub(double a, double b) { return a-b; }\n"
 "double fdiv(double a, double b) { return a/b; }\n"
 "double strequal(unsigned a, unsigned b) { return (a==b)?1.0:0; }\n"
 ;
