template<typename T>
class MatrixStack
{
    public:
        typedef T matrix_type;

    private:
        std::vector<matrix_type> stack;

    public:
        MatrixStack(void)
        {
            stack.push_back(matrix_type::identity());
        }

        void clear(void)
        {
            stack.clear();
            stack.push_back(matrix_type::identity());
        }

        size_t size(void) const
        {
            return stack.size();
        }

        void push(void)
        {
            matrix_type tmp = stack.back(); //required in case the stack's storage gets reallocated
            stack.push_back(tmp);
        }

        bool pop(void)
        {
            if (size() > 1)
            {
                stack.pop_back();
                return true;
            }
            else
            {
                return false;
            }
        }

        void load(const matrix_type& matrix)
        {
            stack.back() = matrix;
        }

        void loadIdentity(void)
        {
            load(matrix_type::identity());
        }

        void loadTransposed(const matrix_type& matrix)
        {
            load(transpose(matrix));
        }

        void mult(const matrix_type& matrix)
        {
            load(stack.back() * matrix);
        }

        void multTransposed(const matrix_type& matrix)
        {
            load(stack.back() * transpose(matrix));
        }

        const matrix_type& get(void) const
        {
            return stack.back();
        }
};
